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
 * @file EcrioPFD_Socket.c
 * @brief This is the implementation of the Ecrio PFD's Socket Module.
 *
 * Design and implementation comments are removed until they can be properly
 * updated.
 */

/* Standard includes. */
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <list>
#include <thread>

using namespace std;

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/select.h>

#include <net/if.h>

#ifdef ENABLE_QCMAPI
#include <android/multinetwork.h>
#endif

/* Include this module's PFD header file. */
#include "EcrioPAL.h"
#include "EcrioPAL_Internal.h"

#ifdef ENABLE_TLS_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#endif /* ENABLE_TLS_OPENSSL */

/** Logging defines and helper functions */
#ifdef ENABLE_LOG
#define SOCKETLOGD(a, b, c, ...)		pal_LogMessage((a), KLogLevelDebug, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define SOCKETLOGI(a, b, c, ...)		pal_LogMessage((a), KLogLevelInfo, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define SOCKETLOGV(a, b, c, ...)		pal_LogMessage((a), KLogLevelVerbose, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define SOCKETLOGW(a, b, c, ...)		pal_LogMessage((a), KLogLevelWarning, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#define SOCKETLOGE(a, b, c, ...)		pal_LogMessage((a), KLogLevelError, KLogComponentPAL, (b), (c),##__VA_ARGS__);

#define SOCKETLOGDUMP(a, b, c, ...)		pal_LogMessageDump((a), KLogLevelInfo, KLogComponentPAL, (b), (c),##__VA_ARGS__);
#else /* ENABLE_LOG */
#define SOCKETLOGD(a, b, c, ...)
#define SOCKETLOGI(a, b, c, ...)
#define SOCKETLOGV(a, b, c, ...)
#define SOCKETLOGW(a, b, c, ...)
#define SOCKETLOGE(a, b, c, ...)

#define SOCKETLOGDUMP(a, b, c, ...)
#endif /* ENABLE_LOG */

/* Definitions. */
#define _ECRIO_SOCKET_IP_SIZE							46			/**< The maximum size of an IPv4/IPv6 IP address, including IPv4 tunneling in IPv6 addresses, plus a null terminator. */
#define _ECRIO_SOCKET_ERROR							-1			/**< Define the socket error constant to be similar to other implementations. */
#define _ECRIO_SOCKET_RECEIVE_BUFFER_SIZE 			4096
#define _ECRIO_SOCKET_CONNECTPOLLBACKOFF			16000
#define _ECRIO_SOCKET_TLS_HASH_SHA1					"sha-1"
#define _ECRIO_SOCKET_TLS_HASH_SHA224				"sha-224"
#define _ECRIO_SOCKET_TLS_HASH_SHA256				"sha-256"
#define _ECRIO_SOCKET_TLS_HASH_SHA384				"sha-384"
#define _ECRIO_SOCKET_TLS_HASH_SHA512				"sha-512"
#define _ECRIO_SOCKET_TLS_HASH_MD5					"md5"
#define _ECRIO_SOCKET_TLS_HASH_MD2					"md2"
#define _ECRIO_SOCKET_TLS_BUF_SIZE					512			/**< Temp buffer size */
#define _ECRIO_SOCKET_TLS_CONNECTION_TIMEOUT		5000	/**< in millisec */
#define _ECRIO_SOCKET_TLS_DEFAULT_CIPHER_SUITES		"AES128-SHA256:AES128-SHA:DES-CBC3-SHA:NULL-SHA:NULL-SHA256:RC4-SHA:AES256-SHA" /**< 
														Default set of ciphers - 
																TLS_RSA_WITH_AES_128_CBC_SHA256, 
																TLS_RSA_WITH_AES_128_CBC_SHA, 
																TLS_RSA_WITH_3DES_EDE_CBC_SHA,
																TLS_RSA_WITH_NULL_SHA,
																TLS_RSA_WITH_NULL_SHA256,
																TLS_RSA_WITH_RC4_128_SHA,
																TLS_RSA_WITH_AES_256_CBC_SHA*/

#define _ECRIO_SOCKET_SIGNAL_EXIT						100
#define _ECRIO_SOCKET_SIGNAL_ERROR					101

typedef void* _ECRIO_SOCKET_SIGNAL_HANDLE;

/* Forward declarations. */
/* messageQ message Structure */
typedef struct
{
	unsigned int uCommand;
	unsigned int uParam;
	void* pParam;
} _EcrioSocketMessageStruct;

/* messageQ structure */
typedef struct
{
	mutex m;
	list <_EcrioSocketMessageStruct> messageQ;
} _EcrioSocketMessageQStruct;

typedef unsigned int(*_EcrioSocketSignalCallback)
(
	_ECRIO_SOCKET_SIGNAL_HANDLE handle,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
);

typedef struct
{
	mutex m;
	condition_variable cond;
	thread hThread;
	_EcrioSocketMessageQStruct messageQStruct;
	_EcrioSocketSignalCallback callback;
	BoolEnum bEnding;
	LOGHANDLE logHandle;
} _EcrioSocketSignalStruct;

/** @struct SocketStruct
 * This is the structure maintained by the Socket Module and represents a
 * Socket instance. Memory is allocated for it when the DRVSocketCreate() 
 * function is called and it is what the Socket handle points to. All
 * subsequent interface functions used for the socket receive this pointer
 * as parameter input.
 */
typedef struct tag_EcrioSocketStruct
{
	SOCKETINSTANCE instance;		/**< Reference pointer to the module that the socket belongs to. */

	ProtocolVersionEnum protocolVersion;
	ProtocolTypeEnum protocolType;
	SocketTypeEnum socketType;
	SocketMediaTypeEnum socketMediaType;

	BoolEnum bEnableGlobalMutex;

	SocketCallbackFnStruct callback;	/**< Callback structure used for storing the callbacks provided by the Calling Component. */

	struct sockaddr_storage local;			/**< Storage of the local host information in socket address format. */
	int localSize;						/**< The actual used size of the local host information. */

	struct sockaddr_storage remote;		/**< Storage of the remote host information in socket address format. */
	int remoteSize;					/**< The actual used size of the remote host information. */

	int socket;						/**< The socket object for this socket instance. */

	unsigned int bufferSize;			/**< The buffer size requested by the caller. */
	char *pRecvBuffer;				/**< Pointer to the socket receive buffer. */

	BoolEnum bClosed;

	char deviceName[128];

#ifdef ENABLE_TLS_OPENSSL
	SSL_CTX *pCtx;					/**< SSL context for TLS type socket */
	SSL *pSsl;						/**< SSL pointer for TLS type socket */
#endif /* ENABLE_TLS_OPENSSL */
	BoolEnum bVerifyPeer;			/**< TRUE if peer certificate verification is set for TLS type socket.Otherwise FALSE */
	BoolEnum bTLSInit;				/**< TRUE if the TLS is properly initialized for TLS type socket.Otherwise FALSE */
	unsigned int uConnTimeoutMilli; 	/**< The value of connection timeout */
	unsigned int uNumOfFingerPrint;
	u_char **ppFingerPrint;

	int syncSendFlags;					/**< Flags to be set when sending data synchronously */

//	thread hAcceptThread;
//	thread hRecvThread;
	pthread_t hAcceptThread;			/**< Handle to the socket accept thread. */
	pthread_t hRecvThread;			/**< Handle to the socket receive thread. */

	mutex m;
} _EcrioSocketStruct;

typedef struct
{
	mutex m;
	list <_EcrioSocketStruct *> l;
} _EcrioSocketStructList;

typedef struct
{
	LOGHANDLE logHandle;
	PALINSTANCE pal;

	_EcrioSocketStructList socketList;

	u_int64 uNetID;
	char deviceName[128];

	_ECRIO_SOCKET_SIGNAL_HANDLE hSocket;
} _EcrioSocketInstanceStruct;

/* MESSAGE QUEUE IMPLEMENTATION - START */
static void _pal_SocketMessageInit
(
	_EcrioSocketMessageQStruct *pMessageQStruct
)
{
	(void)pMessageQStruct;
	//TBD
}

static void _pal_SocketMessageDeinit
(
	_EcrioSocketMessageQStruct *pMessageQStruct
)
{
	(void)pMessageQStruct;
	//TBD
}

static int _pal_SocketMessageAdd
(
	_EcrioSocketMessageQStruct *pMessageQStruct,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
)
{
	lock_guard<mutex> lock(pMessageQStruct->m);
	_EcrioSocketMessageStruct messageStruct = {};
	messageStruct.uCommand = uCommand;
	messageStruct.uParam = uParam;
	messageStruct.pParam = pParam;
	pMessageQStruct->messageQ.push_back(messageStruct);
	return 0;
}

static int _pal_SocketMessageGet
(
	_EcrioSocketMessageQStruct *pMessageQStruct,
	_EcrioSocketMessageStruct *pStruct
)
{
	unique_lock<mutex> lock(pMessageQStruct->m);
	if (pMessageQStruct->messageQ.size() == 0)
	{
		lock.unlock();
		return -1;
	}
	_EcrioSocketMessageStruct messageStruct = {};
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
static void _pal_SocketSignalWorker
(
	void *pStruct
)
{
	_EcrioSocketSignalStruct *h = (_EcrioSocketSignalStruct*)pStruct;
	while (1)
	{
		unique_lock<mutex> lock(h->m);
		h->cond.wait(lock);
		lock.unlock();
		_EcrioSocketMessageStruct messageStruct = {};
		while (_pal_SocketMessageGet(&(h->messageQStruct), &messageStruct) == 0)
		{
			switch (messageStruct.uCommand)
			{
			case _ECRIO_SOCKET_SIGNAL_EXIT:
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

static _ECRIO_SOCKET_SIGNAL_HANDLE _pal_SocketSignalInit
(
	_EcrioSocketSignalCallback pSignalFn,
	LOGHANDLE logHandle,
	u_int32 *pError
)
{
	_EcrioSocketSignalStruct *h = NULL;
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

	h = new (_EcrioSocketSignalStruct);

	h->logHandle = logHandle;
	h->bEnding = Enum_FALSE;

	h->callback = pSignalFn;

	_pal_SocketMessageInit(&(h->messageQStruct));

	h->hThread = thread(_pal_SocketSignalWorker, (void *)h);
	return h;
}

static int _pal_SocketSignalDeinit
(
	_ECRIO_SOCKET_SIGNAL_HANDLE *handle
)
{
	_EcrioSocketSignalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	if (*handle == NULL)
	{
		return -1;
	}
	h = (_EcrioSocketSignalStruct*)*handle;
	h->bEnding = Enum_TRUE;
	_pal_SocketMessageAdd(&(h->messageQStruct), _ECRIO_SOCKET_SIGNAL_EXIT, 0, NULL);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	h->hThread.join();
	_pal_SocketMessageDeinit(&(h->messageQStruct));
	delete h;

	return 0;
}

static int _pal_SocketSignalSend
(
	_ECRIO_SOCKET_SIGNAL_HANDLE handle,
	unsigned int uCommand,
	unsigned int uParam,
	void* pParam
)
{
	_EcrioSocketSignalStruct *h = NULL;
	if (handle == NULL)
	{
		return -1;
	}
	h = (_EcrioSocketSignalStruct*)handle;
	_pal_SocketMessageAdd(&(h->messageQStruct), uCommand, uParam, pParam);
	unique_lock<mutex> lock(h->m);
	h->cond.notify_one();
	lock.unlock();
	return 0;
}
/* SIGNAL QUEUE IMPLEMENTATION - END */

/**
* This function will select TLS method based on the version.
* Currently specific version is not set, Default is recommended.
*/
static const void * _pal_SocketTlsMethod
(
	_EcrioSocketStruct *pSockStruct,
	SocketTLSVersionEnum version
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	const void *method = NULL;

	if (pSockStruct == NULL)
	{
		return NULL;
	}
	i = (_EcrioSocketInstanceStruct *)pSockStruct->instance;


	switch (version)
	{
		case SocketTLSVersion_Default:
		case SocketTLSVersion_TLSv1:
		case SocketTLSVersion_TLSv1_0:
		case SocketTLSVersion_TLSv1_1:
		case SocketTLSVersion_TLSv1_2:
		{
	#ifdef ENABLE_TLS_OPENSSL
			method = SSLv23_client_method();
	#else
			/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
			SOCKETLOGE(i->logHandle, KLogTypeGeneral,
				"%s:%u,  No SSL Library is defined !",
				__FUNCTION__, __LINE__);
	#endif /* ENABLE_TLS_OPENSSL */
		}
		break;

		/** @todo specific version is not set, ignored for now */
		//case SocketTLSVersion_SSLv2:
		//{
		//	/* Not supported method */			
		//}
		//break;

		//case SocketTLSVersion_SSLv3:
		//{
		//	method = SSLv3_client_method();
		//}
		//break;
		
	}

	return method;
}

/**
* This function checks input TLS parameters, initializes the TLS and save 
the TLS object & context for further use.
*/
static unsigned int _pal_SocketTlsInit
(
	_EcrioSocketInstanceStruct *instance, 
	_EcrioSocketStruct *socket, 
	SocketTLSSettingsStruct *pTLSSetting
)
{
	unsigned int returnValue = KPALErrorNone;
	const char* pCiphers = NULL;
#ifdef ENABLE_TLS_OPENSSL
	const SSL_METHOD *method = NULL;
	SSL_CTX *ctx = NULL;
	SSL *ssl = NULL;
#endif /* ENABLE_TLS_OPENSSL */
	int SSLError = 0;

	/** If verify peer is set to Enum_TRUE then CA certifcate is a must parameter*/
	if (pTLSSetting->bVerifyPeer == Enum_TRUE)
	{
		if (pTLSSetting->pCert == NULL)
		{
			return KPALInsufficientParameters;
		}
		socket->bVerifyPeer = Enum_TRUE;
	}
	else
	{
		socket->bVerifyPeer = Enum_FALSE;

		if (pTLSSetting->uNumOfFingerPrint > 0)
		{
			pal_MemoryAllocate(sizeof(u_char *) * pTLSSetting->uNumOfFingerPrint, (void **)&socket->ppFingerPrint);
			for (int j = 0; j < pTLSSetting->uNumOfFingerPrint; j++)
			{
				socket->ppFingerPrint[j] = pal_StringCreate(pTLSSetting->ppFingerPrint[j], pal_StringLength(pTLSSetting->ppFingerPrint[j]));
			}
			socket->uNumOfFingerPrint = pTLSSetting->uNumOfFingerPrint;
		}
		else
		{
			socket->uNumOfFingerPrint = 0;
			socket->ppFingerPrint = NULL;
		}
	}

	/** Set TLS connection timeout, if 0 default value is used*/
	socket->uConnTimeoutMilli = (pTLSSetting->connectionTimeoutMilli > 0) ?
		pTLSSetting->connectionTimeoutMilli : _ECRIO_SOCKET_TLS_CONNECTION_TIMEOUT;

	/** Use default ciphers if not provided */
	pCiphers = (pTLSSetting->pCiphers != NULL) ?
		(const char*)pTLSSetting->pCiphers : _ECRIO_SOCKET_TLS_DEFAULT_CIPHER_SUITES;

#ifdef ENABLE_TLS_OPENSSL
	/** Get SSL method */
	method = (SSL_METHOD*)_pal_SocketTlsMethod(socket, pTLSSetting->socketTLSVersion);
	if (method == NULL)
	{
		SOCKETLOGE(instance->logHandle, KLogTypeGeneral,
			"%s:%u, Wrong version, method is NOT supported!",
			__FUNCTION__, __LINE__);

		returnValue = KPALTLSInternalError;
		goto Error;
	}

	/** Create a new framework to establish TLS/SSL enabled connections	*/
	ctx = SSL_CTX_new(method);   /* Create new context */
	if (ctx == NULL)
	{
		SOCKETLOGE(instance->logHandle, KLogTypeGeneral,
			"%s:%u, Unable to create ssl-context!",
			__FUNCTION__, __LINE__);

		returnValue = KPALTLSInternalError;
		goto Error;
	}

	/** Set the list of ciphers used in negotiation. Items that are not recognized, 
	because the corresponding ciphers are not compiled in or because they are mistyped, 
	are simply ignored. Failure is only flagged if no ciphers could be collected at all.
	*/	
	if (!SSL_CTX_set_cipher_list(ctx, pCiphers)) 
	{
		SOCKETLOGE(instance->logHandle, KLogTypeGeneral,
			"%s:%u, Unable to set cipher-list to ssl!",
			__FUNCTION__, __LINE__);

		returnValue = KPALTLSInternalError;
		goto Error;
	}	
	
	if (pTLSSetting->pCert != NULL)
	{
		/** NOTE:  (optional) This certifcate is set for client authentication which server may requests. 
		For client authentication this certifcate must be a valid certificate which is trusted by server.

		Currently only PEM type supported, where certifcate file itself contains private-key.
		*/
		if (SSL_CTX_use_certificate_file(ctx, (const char*)pTLSSetting->pCert, SSL_FILETYPE_PEM) != 1)
		{
			SSLError = ERR_get_error();
			SSL_load_error_strings();
			SOCKETLOGD(instance->logHandle, KLogTypeGeneral, "SSL certificate Error code:[%lu](%s),%s,%u", 
				SSLError, ERR_error_string(SSLError, NULL), __FUNCTION__, __LINE__);
		}
		else
		{
			if (SSL_CTX_use_PrivateKey_file(ctx, (const char*)pTLSSetting->pCert, SSL_FILETYPE_PEM) == 1)
			{
				SOCKETLOGD(instance->logHandle, KLogTypeGeneral,
					"%s:%u, Successfully loaded private key",
					__FUNCTION__, __LINE__);
			}
		}		
	}

	if (socket->bVerifyPeer == Enum_TRUE)
	{
		/** Set default locations for trusted CA certificates*/
		if (!SSL_CTX_load_verify_locations(ctx,
			(const char*)pTLSSetting->pCert,
			NULL))
		{
			SOCKETLOGE(instance->logHandle, KLogTypeGeneral,
				"%s:%u, Unable to load verify locations to ssl!",
				__FUNCTION__, __LINE__);

			returnValue = KPALTLSInternalError;
			goto Error;
		}

		/** SSL always tries to verify the peer, If the verification process fails, 
		the TLS/SSL handshake is immediately terminated with an alert message containing 
		the reason for the verification failure. The result of the verification can be checked 
		using SSL_get_verify_result() . */
		SSL_CTX_set_verify(ctx,
			SSL_VERIFY_PEER,
			NULL);
	}
	else 
	{
		/** The server certificate will be checked. The result of the certificate verification 
		process can be checked after the TLS/SSL handshake using the SSL_get_verify_result function. 
		The handshake will be continued regardless of the verification result.
		*/
		SSL_CTX_set_verify(ctx,
			SSL_VERIFY_NONE,
			NULL);
	}

	/* Might be required in renegotiation */
	//SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
	//SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

	/** Create a new SSL structure which is needed to hold the data for a TLS/SSL connection*/
	ssl = SSL_new(ctx);
	if (ssl == NULL) 
	{
		SOCKETLOGE(instance->logHandle, KLogTypeGeneral,
			"%s:%u, Unable to create ssl!",
			__FUNCTION__, __LINE__);
		returnValue = KPALTLSInternalError;
		goto Error;
	}

	/** Prepare SSL object to work in client mode*/
	SSL_set_connect_state(ssl);

	/** SSL is initialized, save context & ssl for future use */
	socket->pCtx = ctx;
	socket->pSsl = ssl;
	socket->bTLSInit = Enum_TRUE;

Error:

	if (returnValue != KPALErrorNone)
	{
		/** Error occurred, Free SSL resources */
		if (ssl != NULL)
		{
			SSL_free(ssl);
		}

		if (ctx != NULL)
		{
			SSL_CTX_free(ctx);
		}
	}

#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(instance->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	return returnValue;
}

/**
* This function sets socket descriptor to TLS and does connect/handshake with 
* server in non-blocking mode.
*/
static unsigned int _pal_SocketTlsConnect
(
	_EcrioSocketStruct *pSockStruct
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	unsigned int returnValue = KPALErrorNone;
	unsigned int timeoutmillisec = 0;
#ifdef ENABLE_TLS_OPENSSL
	int res = 0;
	BoolEnum bDone = Enum_FALSE;
	unsigned long flags = 1;
#endif /* ENABLE_TLS_OPENSSL */

	if (pSockStruct == NULL)		
	{
		return KPALInternalError;
	}

	i = (_EcrioSocketInstanceStruct *)pSockStruct->instance;

	/** select timeout */
	timeoutmillisec = pSockStruct->uConnTimeoutMilli;

#ifdef ENABLE_TLS_OPENSSL
	/** Set the socket dexcriptor as the input/output facility for the TLS/SSL (encrypted) side 
	of ssl. If socket is blocking/non-blocking, the ssl will also have blocking/non-blocking behaviour.
	*/
	if (!SSL_set_fd(pSockStruct->pSsl, pSockStruct->socket))
	{
		SOCKETLOGE(i->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) SSL_set_fd() failed, socketError %d",
			__FUNCTION__, __LINE__, pSockStruct, errno);
		returnValue = KPALTLSInternalError;
		goto Error;
	}

	/** Set the underlying socket to non-blocking mode, otherwise SSL_connect would block */
	ioctl(pSockStruct->socket, FIONBIO, &flags);

	ERR_clear_error();
	while (!bDone && ((res = SSL_connect(pSockStruct->pSsl)) <= 0))
	{
#ifdef ANDROID
        unsigned long liberror = 0;
        int err = 0;
        useconds_t connectPollBackoff = 0;
        if (connectPollBackoff > 0)
        {
            usleep(connectPollBackoff);
            connectPollBackoff = connectPollBackoff >= 1000000 ? 1000000 : connectPollBackoff * 2;
        }
        else
        {
            connectPollBackoff = _ECRIO_SOCKET_CONNECTPOLLBACKOFF;
        }

        /** Obtain error code for TLS/SSL I/O operation */
        err = SSL_get_error(pSockStruct->pSsl, res);

        if (connectPollBackoff >= 10000000)
        {
            bDone = Enum_TRUE;
            returnValue = KPALTLSConnectTimeout;

            SOCKETLOGE(i->logHandle, KLogTypeGeneral,
                       "%s:%u,  select() timeout ! timeout(ms) %u result %d, sslerror %d, socketError %d",
                       __FUNCTION__, __LINE__, timeoutmillisec,res, err, errno);
        }
        switch (err)
        {
            case SSL_ERROR_NONE:
            {
                bDone = Enum_TRUE;
            }
            break;

            case SSL_ERROR_WANT_READ:
            {
            }
            break;

            case SSL_ERROR_WANT_WRITE:
            {
            }
            break;

            case SSL_ERROR_SYSCALL:
            {
                if(res < 0)
                {
                    /** handling EINTR explicitly in case system error; EINTR might be handled inside
                    SSL-library depending on compilation switch */
                    if(errno == EINTR)
                    {
                        continue;
                    }
                }

                returnValue = KPALDataSendErrorFatal;
                liberror = ERR_get_error();

                SOCKETLOGE(i->logHandle, KLogTypeGeneral,
                           "%s:%u, SSL read result %d, sslerror %d, liberror %u, socketError %d",
                           __FUNCTION__, __LINE__, res, err, liberror, errno);

            }
            break;

            default:
            {
                /** Most likely a TLS protocol error; Otherwise library error or socket error */
                bDone = Enum_TRUE;
                liberror = ERR_get_error();
                returnValue = KPALTLSInternalError;

                /** Override error code if peer certificate verification fails*/
                if ((liberror == 0x1407E086) || (liberror == 0x14090086))
                {
                    /** 1407E086:
                        SSL routines:
                        SSL2_SET_CERTIFICATE:
                        certificate verify failed
                        14090086:
                        SSL routines:
                        SSL3_GET_SERVER_CERTIFICATE:
                        certificate verify failed */
                    if (pSockStruct->bVerifyPeer == Enum_TRUE)
                    {
                        long res = SSL_get_verify_result(pSockStruct->pSsl);

                        if (res != X509_V_OK)
                        {
                            SOCKETLOGE(i->logHandle, KLogTypeGeneral,
                                       "%s:%u, SSL_get_verify_result failed",
                                       __FUNCTION__, __LINE__);
                            returnValue = KPALTLSPeerCertificateError;
                        }
                    }
                }

                SOCKETLOGE(i->logHandle, KLogTypeGeneral,
                           "%s:%u, SSL Error! result %d, sslerror %d, liberror %u, socketError %d",
                           __FUNCTION__, __LINE__, res, err, liberror, errno);
            }
            break;
        }
#else
		int selectreturn = 0;
		fd_set fdset = { 0 };
		fd_set	errset = { 0 };
		struct timeval tv = { 0 };
		unsigned long liberror = 0;
		int err = 0;
		FD_ZERO(&fdset);
		FD_ZERO(&errset);
		FD_SET(pSockStruct->socket, &fdset);
		FD_SET(pSockStruct->socket, &errset);

		tv.tv_usec = (timeoutmillisec * 1000);

		/** Obtain error code for TLS/SSL I/O operation */
		err = SSL_get_error(pSockStruct->pSsl, res);
		switch (err)
		{
			case SSL_ERROR_NONE:
			{
				bDone = Enum_TRUE;
			}
			break;

			case SSL_ERROR_WANT_READ:
			{
				selectreturn = select( pSockStruct->socket + 1, &fdset, NULL, &errset, &tv);
				if (selectreturn == 0)
				{
					/** Timeout is occurred. Set the error & exit the loop*/
					bDone = Enum_TRUE;
					returnValue = KPALTLSConnectTimeout;

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u,  select() timeout ! timeout(ms) %u result %d, sslerror %d, socketError %d",
						__FUNCTION__, __LINE__, timeoutmillisec,res, err, errno);
				}
				else if (selectreturn < 0)
				{
					/** A failure in select method. Set the error & exit the loop*/
					bDone = Enum_TRUE;
					returnValue = KPALTLSInternalError;

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u, select() failed! result %d, sslerror %d, socketError %d",
						__FUNCTION__, __LINE__, res, err, errno);
				}
				else	
				{
					if ( FD_ISSET(pSockStruct->socket, &fdset) == 0 )
					{
						bDone = Enum_TRUE;
						returnValue = KPALTLSInternalError;
						
						SOCKETLOGE(i->logHandle, KLogTypeGeneral,
							"%s:%u, select() read not set! result %d, sslerror %d, socketError %d",
							__FUNCTION__, __LINE__,res, err, errno);
					}
				}
			}
			break;

			case SSL_ERROR_WANT_WRITE:
			{
				selectreturn = select( pSockStruct->socket + 1, NULL, &fdset, &errset, &tv);
				if (selectreturn == 0)
				{
					/** Timeout is occurred. Set the error & exit the loop*/
					bDone = Enum_TRUE;
					returnValue = KPALTLSConnectTimeout;

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u,  select() timeout ! timeout(ms) %u, result %d, sslerror %d, socketError %d" ,
						__FUNCTION__, __LINE__, timeoutmillisec,res, err, errno);
				}
				else if (selectreturn < 0)
				{
					/** A failure in select method. Set the error & exit the loop*/
					bDone = Enum_TRUE;
					returnValue = KPALTLSInternalError;

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u, select() failed! result %d, sslerror %d, socketError %d",
						__FUNCTION__, __LINE__,res, err, errno);
				}
				else	
				{
					if ( FD_ISSET(pSockStruct->socket, &fdset) == 0 )
					{
						bDone = Enum_TRUE;
						returnValue = KPALTLSInternalError;
						
						SOCKETLOGE(i->logHandle, KLogTypeGeneral,
							"%s:%u, select() write not set! result %d, sslerror %d, socketError %d",
							__FUNCTION__, __LINE__,res, err, errno);
					}
				}
			}
			break;
			
			case SSL_ERROR_SYSCALL:
			{
				if(res < 0) 
				{ 
					/** handling EINTR explicitly in case system error; EINTR might be handled inside 
					SSL-library depending on compilation switch */
					if(errno == EINTR) 
					{
						continue;
					}
				}

				returnValue = KPALDataSendErrorFatal;
				liberror = ERR_get_error();			

				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, SSL read result %d, sslerror %d, liberror %u, socketError %d",
					__FUNCTION__, __LINE__, res, err, liberror, errno);

			}
			break;
			
			default:
			{
				/** Most likely a TLS protocol error; Otherwise library error or socket error */
				bDone = Enum_TRUE;
				liberror = ERR_get_error();
				returnValue = KPALTLSInternalError;
				
				/** Override error code if peer certificate verification fails*/
				if ((liberror == 0x1407E086) || (liberror == 0x14090086))
				{
					/** 1407E086:
						SSL routines:
						SSL2_SET_CERTIFICATE:
						certificate verify failed
						14090086:
						SSL routines:
						SSL3_GET_SERVER_CERTIFICATE:
						certificate verify failed */
					if (pSockStruct->bVerifyPeer == Enum_TRUE)
					{
						long res = SSL_get_verify_result(pSockStruct->pSsl);

						if (res != X509_V_OK)
						{
							SOCKETLOGE(i->logHandle, KLogTypeGeneral,
								"%s:%u, SSL_get_verify_result failed",
								__FUNCTION__, __LINE__);
							returnValue = KPALTLSPeerCertificateError;
						}
					}
				}				
				
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, SSL Error! result %d, sslerror %d, liberror %u, socketError %d",
					__FUNCTION__, __LINE__, res, err, liberror, errno);
			}
			break;

		}
#endif
		ERR_clear_error();
	}

	/** Set the underlying socket back to blocking mode */
	flags = 0;
	ioctl(pSockStruct->socket, FIONBIO, &flags);

Error:

#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(i->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	return returnValue;

}

/**
* This function will check peer certifcate information
* And if peer verification is set, this will verify server certificate if present.
*/
static unsigned int _pal_SocketTlsServerCertificate
(
	_EcrioSocketStruct *pSockStruct	
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	unsigned int returnValue = KPALErrorNone;
#ifdef ENABLE_TLS_OPENSSL
	X509 *cert = NULL;
	const EVP_MD *pHashType = NULL;
	unsigned int uFingerPrintLength = 0;
	unsigned char fingerPrint[EVP_MAX_MD_SIZE];
	unsigned char fingerPrintStr[EVP_MAX_MD_SIZE*4];
#endif /* ENABLE_TLS_OPENSSL */
	unsigned char *pHashValue = NULL;
	BoolEnum bMatchedFingerPrint = Enum_FALSE;
	
	if (pSockStruct == NULL)
	{
		return KPALInternalError;
	}

	i = (_EcrioSocketInstanceStruct *)pSockStruct->instance;

#ifdef ENABLE_TLS_OPENSSL
	/** Get peer certificates (if available) */
	cert = SSL_get_peer_certificate(pSockStruct->pSsl); 
	if (cert != NULL)
	{		
		char buf[_ECRIO_SOCKET_TLS_BUF_SIZE] = { 0 };
		char *line = NULL;

		line = X509_NAME_oneline(X509_get_subject_name(cert), buf, (_ECRIO_SOCKET_TLS_BUF_SIZE - 1));
		
		SOCKETLOGD(i->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) CERTIFICATES SUBJECT %s",
			__FUNCTION__, __LINE__, pSockStruct, line ? buf : "[NONE]");
		
		line = X509_NAME_oneline(X509_get_issuer_name(cert), buf, (_ECRIO_SOCKET_TLS_BUF_SIZE - 1));
		SOCKETLOGD(i->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) CERTIFICATES ISSUER %s",
			__FUNCTION__, __LINE__, pSockStruct, line ? buf : "[NONE]");
		
		if (pSockStruct->bVerifyPeer == Enum_TRUE) 
		{
			long res = SSL_get_verify_result(pSockStruct->pSsl);

			if (res != X509_V_OK )
			{
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, SSL_get_verify_result failed",
					__FUNCTION__, __LINE__);
				returnValue = KPALTLSPeerCertificateError;
				goto Error;
			}
		}
		else
		{
			if (pSockStruct->uNumOfFingerPrint > 0)
			{
				for (int k = 0; k < pSockStruct->uNumOfFingerPrint; k++)
				{
					if (pal_StringNICompare(pSockStruct->ppFingerPrint[k], (const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA1, pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA1)) == 0)
					{
						pHashType = EVP_sha1();
						pHashValue = pSockStruct->ppFingerPrint[k] + pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA1) + 1;
					}
					else if (pal_StringNICompare(pSockStruct->ppFingerPrint[k], (const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA224, pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA224)) == 0)
					{
						pHashType = EVP_sha224();
						pHashValue = pSockStruct->ppFingerPrint[k] + pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA224) + 1;
					}
					else if (pal_StringNICompare(pSockStruct->ppFingerPrint[k], (const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA256, pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA256)) == 0)
					{
						pHashType = EVP_sha256();
						pHashValue = pSockStruct->ppFingerPrint[k] + pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA256) + 1;
					}
					else if (pal_StringNICompare(pSockStruct->ppFingerPrint[k], (const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA384, pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA384)) == 0)
					{
						pHashType = EVP_sha384();
						pHashValue = pSockStruct->ppFingerPrint[k] + pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA384) + 1;
					}
					else if (pal_StringNICompare(pSockStruct->ppFingerPrint[k], (const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA512, pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA512)) == 0)
					{
						pHashType = EVP_sha512();
						pHashValue = pSockStruct->ppFingerPrint[k] + pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_SHA512) + 1;
					}
					else if (pal_StringNICompare(pSockStruct->ppFingerPrint[k], (const u_char *)_ECRIO_SOCKET_TLS_HASH_MD5, pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_MD5)) == 0)
					{
						pHashType = EVP_md5();
						pHashValue = pSockStruct->ppFingerPrint[k] + pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_MD5) + 1;
					}
		//			else if (pal_StringNICompare(pSockStruct->ppFingerPrint[k], (const u_char *)_ECRIO_SOCKET_TLS_HASH_MD2, pal_StringLength((const u_char *)_ECRIO_SOCKET_TLS_HASH_MD2)) == 0)
		//			{
		//				pHashType = EVP_md2();
		//				pHashValue = pSockStruct->pFingerPrint + ppFingerPrint[k]((const u_char *)_ECRIO_SOCKET_TLS_HASH_MD2) + 1;
		//			}
					else
					{
						SOCKETLOGE(i->logHandle, KLogTypeGeneral,
							"%s:%u, unsupported HASH type",
							__FUNCTION__, __LINE__);
						continue;
					}

					pal_MemorySet(fingerPrint, 0, EVP_MAX_MD_SIZE);
					if (!X509_digest(cert, pHashType, fingerPrint, &uFingerPrintLength))
					{
						SOCKETLOGE(i->logHandle, KLogTypeGeneral,
							"%s:%u, X509_digest() error",
							__FUNCTION__, __LINE__);
						continue;
					}

					pal_MemorySet(fingerPrintStr, 0, EVP_MAX_MD_SIZE*4);
					pal_StringSNPrintf((char *)fingerPrintStr, EVP_MAX_MD_SIZE*4, "%02X", fingerPrint[0]);
					for (int j = 1; j < uFingerPrintLength; j++)
					{
						unsigned char hexStr[3];
						pal_StringSNPrintf((char *)hexStr, 3, "%02X", fingerPrint[j]);
						pal_StringNConcatenate(fingerPrintStr, (EVP_MAX_MD_SIZE*4 - (3 * j)), (const u_char *)":", 1);
						pal_StringNConcatenate(fingerPrintStr, (EVP_MAX_MD_SIZE*4 - ((3 * j) + 1)), hexStr, 2);
					}

					SOCKETLOGD(i->logHandle, KLogTypeGeneral,
						"%s:%u,fingerprint from SDP %s",
						__FUNCTION__, __LINE__, pHashValue);

					SOCKETLOGD(i->logHandle, KLogTypeGeneral,
						"%s:%u,fingerprint from certificates %s",
						__FUNCTION__, __LINE__, fingerPrintStr);

					if (pal_StringNICompare(pHashValue, fingerPrintStr, pal_StringLength(fingerPrintStr)) != 0)
					{
						SOCKETLOGE(i->logHandle, KLogTypeGeneral,
							"%s:%u, miss-matched fingerprint",
							__FUNCTION__, __LINE__);
						continue;
					}
					else
					{
						bMatchedFingerPrint = Enum_TRUE;
						break;
					}
				}
				
				if (bMatchedFingerPrint != Enum_TRUE)
				{
					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u, miss-matched fingerprint",
						__FUNCTION__, __LINE__);
					returnValue = KPALTLSPeerCertificateError;
					goto Error;
				}
			}
		}

		/** @todo verify host?
		 verify expiry date of certficate ? */
	}
	else 
	{
		SOCKETLOGD(i->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) NO CERTIFICATES",
			__FUNCTION__, __LINE__, pSockStruct);
		returnValue = KPALTLSPeerCertificateError;
		goto Error;
	}

Error:
	if (cert != NULL) 
	{
		X509_free(cert);
	}
#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(i->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	return returnValue;
}

/**
* This function will write buffer through TLS in blocking mode.
* 
*/
unsigned int _pal_SocketTlsWrite
(
	_EcrioSocketStruct *pSockStruct,
	const char* buffer,
	int bufsize,
	int* pSSLReturn
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	unsigned int returnValue = KPALErrorNone;
	int res = 0;
	bool bContinue = false;

	if ((pSockStruct == NULL) ||
		(pSSLReturn == NULL))
	{
		return KPALInternalError;
	}

	i = (_EcrioSocketInstanceStruct *)pSockStruct->instance;

#ifdef ENABLE_TLS_OPENSSL
	/** SSL write on blocking socket */
	do
	{
		ERR_clear_error();
		res = SSL_write(pSockStruct->pSsl, buffer, bufsize);
		if (res <= 0) 
		{
			unsigned long liberror = 0;
			int err = SSL_get_error(pSockStruct->pSsl, res);		
			
			switch (err) 
			{
				case SSL_ERROR_NONE:
				break;

				case SSL_ERROR_ZERO_RETURN:
				{
					/** The TLS/SSL connection has been closed.*/
					returnValue = KPALEntityClosed;
				}
				break;

				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
				{
					/** There is data pending, re-invoke (in case non-blocking)*/
					returnValue = KPALDataSendErrorBenign;
				}
				break;

				case SSL_ERROR_SYSCALL:
				{
					if(res < 0) 
					{ 
						/** handling EINTR explicitly in case system error; EINTR might be handled inside 
						SSL-library depending on compilation switch */
						if(errno == EINTR) 
						{
							bContinue = true;
							continue;
						}
					}

					returnValue = KPALDataSendErrorFatal;
					liberror = ERR_get_error();			

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u, SSL write result %d, sslerror %d, liberror %u, socketError %d",
						__FUNCTION__, __LINE__, res, err, liberror, errno);

				}
				break;
					
				default:
				{
					/** A failure in SSL method, could be a protocol error, library error or a socket error.
					The OpenSSL error queue contains more information on the error. */
					returnValue = KPALDataSendErrorFatal;
					liberror = ERR_get_error();			

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u, SSL write result %d, sslerror %d, liberror %u, socketError %d",
						__FUNCTION__, __LINE__, res, err, liberror, errno);
				}
				break;

			}
		}
	}while(bContinue == true);

#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(i->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	*pSSLReturn = res;
	return returnValue;
}

/**
* This function will read buffer through TLS in blocking mode.
*
*/
unsigned int _pal_SocketTlsRead
(
	_EcrioSocketStruct *pSockStruct,
	const char* buffer,
	int bufsize,
	int* pSSLReturn
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	unsigned int returnValue = KPALErrorNone;
	int res = 0;
	bool bContinue = false;

	if ( (pSockStruct == NULL) ||
		(pSSLReturn == NULL) )
	{
		return KPALInternalError;
	}

	i = (_EcrioSocketInstanceStruct *)pSockStruct->instance;

#ifdef ENABLE_TLS_OPENSSL
	/** SSL read on blocking socket */
	do
	{
		ERR_clear_error();
		res = SSL_read(pSockStruct->pSsl, (void*)buffer, bufsize);
		if (res <= 0) 
		{
			unsigned long liberror = 0;
			int err = SSL_get_error(pSockStruct->pSsl, res);		
			
			switch (err) 
			{
				case SSL_ERROR_NONE:
				break;

				case SSL_ERROR_ZERO_RETURN:
				{
					/** The TLS/SSL connection has been closed.*/
					returnValue = KPALEntityClosed;
				}
				break;

				case SSL_ERROR_WANT_WRITE:
				case SSL_ERROR_WANT_READ:
				{
					/** There is data pending, re-invoke (in case non-blocking)*/
					returnValue = KPALDataReceiveErrorBenign;
				}
				break;
				
				case SSL_ERROR_SYSCALL:
				{
					if(res < 0) 
					{ 
						/** handling EINTR explicitly in case system error; EINTR might be handled inside 
						SSL-library depending on compilation switch */
						if(errno == EINTR) 
						{
							bContinue = true;
							continue;
						}
					}

					returnValue = KPALDataSendErrorFatal;
					liberror = ERR_get_error();			

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u, SSL read result %d, sslerror %d, liberror %u, socketError %d",
						__FUNCTION__, __LINE__, res, err, liberror, errno);

				}
				break;
				
				default:
				{
					/** A failure in the SSL library occurred, could be a protocol error, library error or a socket error.
					The OpenSSL error queue contains more information on the error. */
					returnValue = KPALDataReceiveErrorFatal;
					liberror = ERR_get_error();

					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u,SSL read result %d, sslerror %d, liberror %u, socketError %d",
						__FUNCTION__, __LINE__, res, err, liberror, errno);
				}
				break;

			}
		}
	}while(bContinue == true);

#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(i->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	*pSSLReturn = res;
	return returnValue;
}


/**
 * Sets host information in to a given socket address structure by matching
 * the desired IP and port to a specific interface on the device. Can be used
 * for both IPv4 and IPv6.
 *
 * @param[in] pSockStruct	Handle of Socket.
 * @param[in] pIP			An IP address set.
 * @param[in] uPort			A port number to set.
 * @param[in] bBind			Indicates if the structure will be used in a bind operation.
 * @param[out] addr			The address structure to set.
 * @param[out] addrLen		The size of the address structure that is used.
 * @return Proper PFD error value.
 */
static unsigned int _pal_SocketSetHostInfo
(
	_EcrioSocketStruct *pSockStruct,
	const char* pIP,
	unsigned short uPort,
	BoolEnum bBind,
	struct sockaddr_storage *addr,
	int *addrLen
)
{
	_EcrioSocketInstanceStruct *i;
	char sPort[6] = {0};
	int family = 0;
	int socktype = 0;
	int protocol = IPPROTO_UDP;
	unsigned int returnValue = KPALErrorNone;
	struct addrinfo hints;
	struct addrinfo *pRes = NULL;
	struct addrinfo *pRessave = NULL;
	BoolEnum bFound = Enum_FALSE;
	int socketReturn = 0;

	if (pSockStruct == NULL)
	{
		return KPALInternalError;
	}

	i = (_EcrioSocketInstanceStruct *)pSockStruct->instance;

	pal_StringSNPrintf((char*)sPort, 6, "%u", uPort);

	/* Set the family variable from SocketStruct. If version is not IPv6 or
	   IPv6 Dual Stack then IPv4 will be set. */
	family = ( (pSockStruct->protocolVersion == ProtocolVersion_IPv6) ||
			   (pSockStruct->protocolVersion == ProtocolVersion_IPv6_DualStack) )
			   ? AF_INET6 : AF_INET;

	/* Set the socket type. */
	switch(pSockStruct->protocolType)
	{
		case ProtocolType_TCP_Client:
		case ProtocolType_TCP_Server:
		case ProtocolType_TLS_Client:
		{
			socktype = SOCK_STREAM;
			protocol = IPPROTO_TCP;
		}
		break;

		case ProtocolType_UDP:
		default:
		{
			socktype = SOCK_DGRAM;
			protocol = IPPROTO_UDP;
		}
		break;
	}

#ifdef ENABLE_QCMAPI
	{
		struct ifaddrs * ifAddrStruct = NULL;
		struct ifaddrs * ifa = NULL;
		void * tmpAddrPtr = NULL;

		android_getifaddrs(&ifAddrStruct);
		for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr->sa_family==AF_INET6)
			{
				tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
				char addressBuffer[INET6_ADDRSTRLEN];
				pal_MemorySet((void *)addressBuffer, 0, INET6_ADDRSTRLEN);
				inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
				SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tifa_name=%s, ip=%s",__FUNCTION__,__LINE__, ifa->ifa_name, addressBuffer);
			}
			else
			{
				tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
				char addressBuffer[_ECRIO_SOCKET_IP_SIZE];
				pal_MemorySet((void *)addressBuffer, 0, _ECRIO_SOCKET_IP_SIZE);
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, _ECRIO_SOCKET_IP_SIZE);
				SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tifa_name=%s, ip=%s",__FUNCTION__,__LINE__, ifa->ifa_name, addressBuffer);
			}
		}
		if (ifAddrStruct != NULL)
		{
			android_freeifaddrs(ifAddrStruct);
		}
	}
#endif

	/* Initialize the hints memory to 0. */
	//returnValue = DRVMemorySet((void *)&hints, 0, sizeof(struct addrinfo));
	pal_MemorySet((void *)&hints, 0, sizeof(struct addrinfo));
	/* Ignoring the return value. */

	/* Setup the hints structure to look for the specific constraints we are
	   interested in. */
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	hints.ai_protocol = protocol;

	/* @todo For the time being, we are allowing names to be resolved, but the
	   Socket interfaces do not allow this and we don't really accomodate it
	   fully with IP field sizes limited to _ECRIO_SOCKET_IP_SIZE. */
	//hints.ai_flags = AI_NUMERICHOST;
	hints.ai_flags = 0;

	if (bBind)
	{
		/* We are dealing with the local IP which will be bound via bind. */
		hints.ai_flags |= AI_PASSIVE;
	}

	/* Call getaddrinfo() which translates a host name to an address. */
	socketReturn = getaddrinfo(pIP, sPort, &hints, &pRes);
	if (socketReturn != 0)
	{
		SOCKETLOGW(i->logHandle,KLogTypeGeneral,
			"%s:%u, getaddrinfo() failed, socketReturn %u socketError %d", 
			__FUNCTION__,__LINE__,socketReturn, errno);

		/* @todo Consider handling errors such as "try again" and see when the
		   API should be called again, either here or via the Calling
		   Component. */

		return KPALInternalError;
	}
	else
	{
		pRessave = pRes;

		/* Loop through the linked list of addrinfo structures that getaddrinfo
		   returns until we find the one that matches our constraints. */
		/* @todo Find out if it is possible to end up with multiple structures
		   that satisfy the same constraints. If so, what do we do? */
		while (pRes)
		{
			if ( (family == pRes->ai_family) &&
				 (socktype == pRes->ai_socktype) &&
				 (protocol == pRes->ai_protocol) )
			{
				/* Copy the address to the destination structure. */
				//returnValue = DRVMemoryCopy(addr, pRes->ai_addr, (unsigned int)pRes->ai_addrlen);
				memcpy(addr, pRes->ai_addr, (unsigned int)pRes->ai_addrlen);
				/* Ignoring the return value. */

				/* Hold on to the actual size (the used part of the
				   sockaddr structure) so that we don't need to do all this
				   work again. */
				*addrLen = (int)pRes->ai_addrlen;

				bFound = Enum_TRUE;
				break;
			}
			else
			{
				pRes = pRes->ai_next;
			}
		}

		freeaddrinfo(pRessave);
	}

	if (bFound != Enum_TRUE)
	{
		/* If family and the socket type does not match with the existing one
		   then this error will be returned. This error can occur only when
		   any one of the family or socket type is missing. */
		return KPALEntityNotFound;
	}

	return KPALErrorNone;
}

/**
 * Utility function to close the socket and cleanup the internal socket
 * structures.
 */
static unsigned int _pal_SocketClose
(
	_EcrioSocketInstanceStruct *i,
	_EcrioSocketStruct *s
)
{
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;
	void* value_ptr;
#ifdef ENABLE_TLS_OPENSSL
	SSL *ssl = NULL;
	SSL_CTX *ctx = NULL;
	int sslReturn = 0;
#endif /* ENABLE_TLS_OPENSSL */

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	unique_lock<mutex> lock(s->m);

	/* Indicate the the user is closing the socket, to prevent any further
	   callbacks. */
	s->bClosed = Enum_TRUE;

	/** This function calls the Winsock closesocket() API to actually close the
		socket. */
	if (s->socket > _ECRIO_SOCKET_ERROR)
	{
		int flags, oflags;

		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_GETFL) is calling",__FUNCTION__,__LINE__);
		socketReturn = flags = fcntl(s->socket, F_GETFL, 0);
		if (socketReturn == -1)
		{
			SOCKETLOGE(i->logHandle, KLogTypeGeneral,
				"%s:%u, fcntl(F_GETFL) failed, res %d errno %d",
				__FUNCTION__, __LINE__, socketReturn, errno);
		}
		oflags = flags;
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, socketReturn);

		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_SETFL) is calling, flags | O_NONBLOCK =%d",__FUNCTION__,__LINE__, flags | O_NONBLOCK);
		socketReturn = fcntl(s->socket, F_SETFL, flags | O_NONBLOCK);
		if (socketReturn == -1)
		{
			SOCKETLOGE(i->logHandle, KLogTypeGeneral,
				"%s:%u, fcntl(F_SETFL) failed, res %d errno %d",
				__FUNCTION__, __LINE__, socketReturn, errno);
		}
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, socketReturn);

		/** For TLS client, save SSL resources here. These would be released after recieve thread is exit */
		if (s->protocolType == ProtocolType_TLS_Client)
		{
#ifdef ENABLE_TLS_OPENSSL
			ssl = s->pSsl;
			ctx = s->pCtx;

			/** SSL handshake shutdown */
			if (ssl != NULL)
			{
				sslReturn = SSL_shutdown(ssl);
				/** Ignore the return value. */
				SOCKETLOGD(i->logHandle, KLogTypeGeneral, "%s:%u, SSL shutdown, sslReturn %d", __FUNCTION__, __LINE__, sslReturn);
			}
#else
			/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
			SOCKETLOGE(i->logHandle, KLogTypeGeneral,
				"%s:%u,  No SSL Library is defined !",
				__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */
		}
		socketReturn = shutdown(s->socket, SHUT_RDWR);
		/* Ignore the return value. */
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u, shutdown, socketReturn %d", 
			__FUNCTION__,__LINE__,socketReturn);

	}
	lock.unlock();

	if (s->hRecvThread != 0)
	{
		if (s->bEnableGlobalMutex)
		{
			/* Global mutex unlocked. */
			//DRVGlobalMutexUnlock();
			pal_MutexGlobalUnlock(i->pal);
		}

		/** Wait for the Socket Receive thread to terminate. */
		pthread_join(s->hRecvThread, &value_ptr);
		/* @todo Note that waiting for the thread to exit is an infinite operation. */

		if (s->bEnableGlobalMutex)
		{
			/* Global mutex locked. */
			//DRVGlobalMutexLock();
			pal_MutexGlobalLock(i->pal);
		}

		s->hRecvThread = 0;
	}

	if (s->hAcceptThread != 0)
	{
		if (s->bEnableGlobalMutex)
		{
			/* Global mutex unlocked. */
			//DRVGlobalMutexUnlock();
			pal_MutexGlobalUnlock(i->pal);
		}

		/** Wait for the Socket Accept thread to terminate. */
		pthread_join(s->hAcceptThread, &value_ptr);
		/* @todo Note that waiting for the thread to exit is an infinite operation. */

		if (s->bEnableGlobalMutex)
		{
			/* Global mutex locked. */
			//DRVGlobalMutexLock();
			pal_MutexGlobalLock(i->pal);
		}

		s->hAcceptThread = 0;
	}

	if (s->uNumOfFingerPrint > 0)
	{
		for (int j = 0; j < s->uNumOfFingerPrint; j++)
		{
			pal_MemoryFree((void **)&s->ppFingerPrint[j]);
		}
		pal_MemoryFree((void **)&s->ppFingerPrint);
	}
	
#ifdef ENABLE_TLS_OPENSSL
	/** Free SSL resources*/
	if (ssl != NULL)
	{
		SSL_free(ssl);
		s->pSsl = NULL;
	}

	if (ctx != NULL)
	{
		SSL_CTX_free(ctx);
		s->pCtx = NULL;
	}

#endif /* ENABLE_TLS_OPENSSL */

	if (s->pRecvBuffer != NULL)
	{
		/* Free the memory allocated for the receive buffer. */
		pal_MemoryFree((void **)&s->pRecvBuffer);
		/* Ignoring the return value. */
	}

	/** Free the memory allocated for the Socket instance. */

	delete s;
//	if (s != NULL)
//	{
//		pal_MemoryFree((void **)&s);
//	}
	/* Ignoring the return value. */

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u", __FUNCTION__,__LINE__);
	return KPALErrorNone;
}

static int _pal_socketGetSocket
(
	_EcrioSocketInstanceStruct *i,
	_EcrioSocketStruct **s
)
{
	unique_lock<mutex> lock(i->socketList.m);
	if (i->socketList.l.size() == 0)
	{
		lock.unlock();
		return -1;
	}

	*s = i->socketList.l.front();
	i->socketList.l.pop_front();
	lock.unlock();

	return 0;
}


/**
 * Converts the binary form of the address to the text form. This function
 * returns the IP address in printable character format from the delivered
 * SOCKADDR structure. Can be used for both the IPv4 and IPv6 families.
 *
 * @param[in] af		Which family it belongs to.
 * @param[in] pSrc		The SOCKADDR_STORAGE structure.
 * @param[out] pDst		The IP address.
 * @param[in] cnt		Length of the pDst buffer.
 * @param[out] pPort	The Port number.
 * @return Proper PAL error value.
 */
unsigned int _pal_socketSockAddressToHostName
(
	_EcrioSocketInstanceStruct *i,
	int af,
	const void *pSrc,
	char *pDst,
	socklen_t cnt,
	unsigned short *pPort
)
{
	unsigned int error = KPALErrorNone;
	int socketReturn = 0;

	/* Remove compiler warning, m is used if logging macros are defined. */
	i = i;

	switch (af)
	{
		case AF_INET6:
		{
			struct sockaddr_in6 *pIpv6 = NULL;

			socketReturn = getnameinfo((struct sockaddr *)pSrc, sizeof(struct sockaddr_in6), pDst, cnt, NULL, 0, NI_NUMERICHOST);
			if (socketReturn == 0)
			{
				/* Type cast from SOCKADDR_STORAGE to IPV6 [sockaddr_in6] structure. */
				pIpv6 = (struct sockaddr_in6 *)pSrc;
				*pPort = ntohs(pIpv6->sin6_port);
			}
			else
			{
				SOCKETLOGW(i->logHandle, KLogTypeGeneral,
					"%s:%u, getnameinfo(IPv6) failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, socketReturn, errno);

				return KPALInternalError;
			}
		}
		break;

		case AF_INET:
		default:
		{
			struct sockaddr_in *pIpv4 = NULL;

			socketReturn = getnameinfo((struct sockaddr *)pSrc, sizeof(struct sockaddr_in), pDst, cnt, NULL, 0, NI_NUMERICHOST);
			if (socketReturn == 0)
			{
				/* Type cast from SOCKADDR_STORAGE to IPV4 [sockaddr_in] structure. */
				pIpv4 = (struct sockaddr_in *)pSrc;
				*pPort = ntohs(pIpv4->sin_port);
			}
			else
			{
				SOCKETLOGW(i->logHandle, KLogTypeGeneral,
					"%s:%u, getnameinfo(IPv4) failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, socketReturn, errno);

				return KPALInternalError;
			}
		}
		break;
	}

	return error;
}

static unsigned int _pal_SocketHandler
(
	_ECRIO_SOCKET_SIGNAL_HANDLE handle,
	unsigned int uCommand,
	unsigned int uParam,
	void *pParam
)
{
	_EcrioSocketStruct *s = (_EcrioSocketStruct *)pParam;
	_EcrioSocketInstanceStruct *i = (_EcrioSocketInstanceStruct *)s->instance;
	BoolEnum bEnableGlobalMutex = Enum_FALSE;
	SocketErrorCallback callback = NULL;
	void *pCallbackData = NULL;
	unsigned int error = KPALErrorNone;

	if (s == NULL)
	{
		return KPALInvalidHandle;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	if(uCommand == _ECRIO_SOCKET_SIGNAL_ERROR)
	{
		bEnableGlobalMutex = s->bEnableGlobalMutex;
		callback = s->callback.pErrorCallbackFn;							
		pCallbackData = s->callback.pCallbackFnData;
		error = uParam;

		if (callback == NULL)
		{
			return KPALInvalidHandle;
		}

		if (bEnableGlobalMutex)
		{
			/* Global mutex locked. */
			//DRVGlobalMutexLock();
			pal_MutexGlobalLock(i->pal);
		}

		if (s->bClosed)
		{
			pal_MutexGlobalUnlock(i->pal);
			return KPALInternalError;;
		}

		/** Send the error from one of the sockets to the
			Calling Component. */
		callback(
			s,
			pCallbackData,
			error);

		if (bEnableGlobalMutex)
		{
			/* Global mutex unlocked. */
			//DRVGlobalMutexUnlock();
			pal_MutexGlobalUnlock(i->pal);
		}

		callback = NULL;
		pCallbackData = NULL;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u", __FUNCTION__,__LINE__);

	return KPALErrorNone;
}

static void *_pal_SocketRecvWorker
(
	void *lpParam
)
{
	_EcrioSocketInstanceStruct *i;
	_EcrioSocketStruct *s;
	int socketReturn = 0;
	int socketError = 0;
	unsigned int returnValue = KPALErrorNone;
	SocketReceiveStruct tReceive = {0};
	char* pFromIP = NULL;
	unsigned short fromPort = 0;
#ifdef ENABLE_TLS_OPENSSL
	SSL *ssl = NULL;
	SSL_CTX *ctx = NULL;
	int sslReturn = 0;
#endif /* ENABLE_TLS_OPENSSL */

	/* The parameter does not need to be checked for NULL because we require
	   the init function to properly set it. */
	s = (_EcrioSocketStruct *)lpParam;

	i = (_EcrioSocketInstanceStruct *)s->instance;

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u",__FUNCTION__,__LINE__);

	/** The thread implements an infinite while loop, in which the socket
		recv() or recvfrom() function will be invoked and block until data
		is actually received over the socket. If the socket is closed, the
		functions will return an error which can be handled to exit the
		loop and hence the thread. Other errors, are processed as needed. */
	for ( ; ; )
	{
//		unique_lock<mutex> lockBuffer(s->m);
		/** Clear the buffer before receiving. */
		pal_MemorySet(s->pRecvBuffer, 0, s->bufferSize + 1);
//		lockBuffer.unlock();

		SOCKETLOGI(i->logHandle,KLogTypeGeneral,"%s:%u",__FUNCTION__,__LINE__);

		if (s->protocolType == ProtocolType_TLS_Client)
		{
			returnValue = _pal_SocketTlsRead(s, s->pRecvBuffer, s->bufferSize, &socketReturn );
			if (socketReturn <= 0)
			{
				socketReturn = _ECRIO_SOCKET_ERROR;
			}
		}
		else
		{
			socketReturn = recv(s->socket, s->pRecvBuffer, s->bufferSize, 0);
		}

		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x, %u) recvfrom, socketReturn %d",
			__FUNCTION__,__LINE__, s, s->protocolType, socketReturn);

		unique_lock<mutex> lock(s->m);
		if (s->bClosed)
		{
			lock.unlock();
			break;
		}

		if (socketReturn == _ECRIO_SOCKET_ERROR)
		{
			/* Conserve the error value. */
			socketError = errno;

			if (socketError == EINTR)
			{
				/** The blocking function was interrupted by a signal. */

				/** Try again. */

				lock.unlock();
				continue;
			}
			/* else evaluate using the normal error flow. */

			if ( (s->protocolType == ProtocolType_UDP) &&
				 ( (socketError == ECONNRESET) || (socketError == ECONNREFUSED) ) )
			{
				/** If there was a ECONNRESET error it usually means that a
					send() or sendto() was called for a non-existent peer.
					For UDP, this is usually for sending functions and not
					fatal, we will send a benign error message to the Calling
					Component via the module's error callback. */
				/** If there was a ECONNREFUSED error, for UDP, it means that
					the remote peer is available but not taking incoming data,
					generally caused by send() or sendto(). */

				lock.unlock();
				continue;
			}
			else if ( (socketError == EWOULDBLOCK) ||
					  (socketError == EMSGSIZE) ||
					  (socketError == ETIMEDOUT)  ||
					  ((s->protocolType == ProtocolType_TLS_Client) && (returnValue == KPALDataReceiveErrorBenign)))
			{
				/** Treating these errors as benign and allow the Calling
					Component to decide if the socket should be closed or
					not. Typically, these errors can be ignored and the
					higher level protocols will attempt to retry. */

				SOCKETLOGD(i->logHandle,KLogTypeGeneral,
					"%s:%u,(0x%08x, %u) benign receive error, socketError %d", 
					__FUNCTION__,__LINE__,s, s->protocolType, socketError);

				lock.unlock();

				if (s->callback.pReceiveCallbackFn != NULL)
				{
					tReceive.handle = s;
					tReceive.pReceiveData = NULL;
					tReceive.receiveLength = 0;
					tReceive.pFromIP = NULL;
					tReceive.fromPort = 0;
					tReceive.uTTL_HL = 0;
					tReceive.result = KPALDataReceiveErrorBenign;

					if (s->bEnableGlobalMutex)
					{
						/* Global mutex locked. */
						//DRVGlobalMutexLock();
						pal_MutexGlobalLock(i->pal);
					}

					s->callback.pReceiveCallbackFn(
						s->callback.pCallbackFnData,
						&tReceive);

					if (s->bEnableGlobalMutex)
					{
						/* Global mutex unlocked. */
						//DRVGlobalMutexUnlock();
						pal_MutexGlobalUnlock(i->pal);
					}
				}

				/* Continue the while loop. */
				continue;
			}
			else
			{
				/** An error indicating an unrecoverable event or the end of
					the socket was encountered. The Calling Component should
					close the socket and either give up or try again. */

				SOCKETLOGW(i->logHandle,KLogTypeGeneral,
					"%s:%u,(0x%08x, %u) terminating receive error, socketError %d",
					__FUNCTION__,__LINE__, s, s->protocolType, socketError);

				lock.unlock();

				/** Post a message to the message queue. */
				_pal_SocketSignalSend(i->hSocket, _ECRIO_SOCKET_SIGNAL_ERROR, KPALEntityClosed, s);

				/* Exit the while loop. */
				break;
			}
		}
		else if ( (socketReturn == 0) &&
				  (s->protocolType == ProtocolType_TCP_Client) )
		{
			/** The Socket is a TCP Client and the receive function indicated
				that 0 bytes were received. The Calling Component should close
				the Socket and either give up or try again because the
				connection has been closed. UDP packets can be zero length. */

			SOCKETLOGW(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x, %u) terminating receive error, socketReturn %d",
				__FUNCTION__,__LINE__, s, s->protocolType, socketReturn);

			lock.unlock();

			/** Post a message to the message queue. */
			_pal_SocketSignalSend(i->hSocket, _ECRIO_SOCKET_SIGNAL_ERROR, KPALEntityClosed, s);

			/* Exit the while loop. */
			break;
		}
		else
		{		
			if (socketReturn == 0)
			{
				tReceive.handle = s;
				tReceive.pReceiveData = NULL;
				tReceive.receiveLength = 0;
				tReceive.pFromIP = (const unsigned char*)pFromIP;
				tReceive.fromPort = fromPort;
				tReceive.uTTL_HL = 0;
				tReceive.result = KPALErrorNone;

				lock.unlock();

				if (s->bEnableGlobalMutex)
				{
					/* Global mutex locked. */
					//DRVGlobalMutexLock();
					pal_MutexGlobalLock(i->pal);
				}

				/** If the length of the received data is 0, inform the Calling
					Component but set the buffer pointer to NULL because there
					is no valid buffer to point to! 0 length packets are legal
					for UDP. */
				s->callback.pReceiveCallbackFn(
					s->callback.pCallbackFnData,
					&tReceive);

				if (s->bEnableGlobalMutex)
				{
					/* Global mutex unlocked. */
					//DRVGlobalMutexUnlock();
					pal_MutexGlobalUnlock(i->pal);
				}
			}
			else
			{
				tReceive.handle = s;
				tReceive.pReceiveData = (unsigned char *)s->pRecvBuffer;
				tReceive.receiveLength = socketReturn;
				tReceive.pFromIP = (const unsigned char*)pFromIP;
				tReceive.fromPort = fromPort;
				tReceive.uTTL_HL = 0;
				tReceive.result = KPALErrorNone;

				lock.unlock();

				if (s->bEnableGlobalMutex)
				{
					/* Global mutex locked. */
					//DRVGlobalMutexLock();
					pal_MutexGlobalLock(i->pal);
				}

				/** If there is valid received data, the normal case, inform
					the Calling Component via the receive callback. */
				s->callback.pReceiveCallbackFn(
					s->callback.pCallbackFnData,
					&tReceive);

				if (s->bEnableGlobalMutex)
				{
					/* Global mutex unlocked. */
					//DRVGlobalMutexUnlock();
					pal_MutexGlobalUnlock(i->pal);
				}
			}
		}
	}

	unique_lock<mutex> lockClosed(s->m);

	while(1)
	{
		socketReturn = close(s->socket);
		if (socketReturn == _ECRIO_SOCKET_ERROR)
		{
			if (errno == EINTR)
			{
				SOCKETLOGE(i->logHandle,KLogTypeGeneral,
					"%s:%u, close, socketReturn %d EINTR", 
					__FUNCTION__,__LINE__,socketReturn);

				continue;
			}
		}

		break;
	}
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,
		"%s:%u, close, socketReturn %d", 
		__FUNCTION__,__LINE__,socketReturn);

	s->socket = _ECRIO_SOCKET_ERROR;

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u)",__FUNCTION__,__LINE__ );
	
	lockClosed.unlock();

	pthread_exit(NULL);	
	/* Should never get here. */
	return NULL;
}

static void *_pal_SocketAcceptWorker
(
	void *lpParam
)
{
	_EcrioSocketInstanceStruct *i;
	_EcrioSocketStruct *s;
	int socketReturn = 0;
	int socketError = 0;
	_EcrioSocketStruct *hIncoming = NULL;
	int IncomingSocket;
	unsigned int returnValue = KPALErrorNone;
	struct sockaddr_storage incomingAddr = {0};
	int	incomingLength;
	char incomingIP[_ECRIO_SOCKET_IP_SIZE] = {0};
	ProtocolVersionEnum incomingIPVersion;
	unsigned short incomingPort = 0;
	bool bError;
	int res = 0;

	/* This variable must be defined with the Ecrio data type. */
	BoolEnum bAccept;

	/* The parameter does not need to be checked for NULL because we require
	   the init function to properly set it. */
	s = (_EcrioSocketStruct *)lpParam;

	i = (_EcrioSocketInstanceStruct *)s->instance;

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	/** This thread loops listening for incoming TCP Client connections. When
		a connection request comes in, it is verified with the Calling
		Component and accepted as a normal TCP Client Socket. The thread loop
		will exit when the socket is closed. */
	for ( ; ; )
	{
		incomingLength = sizeof(struct sockaddr_storage);

		/** Block while waiting to accept an incoming connection. */
		IncomingSocket = accept(s->socket, (struct sockaddr *)&incomingAddr, (socklen_t*)&incomingLength);
		if (IncomingSocket == _ECRIO_SOCKET_ERROR)
		{
			unique_lock<mutex> lockError(s->m);
			/* Conserve the error value. */
			socketError = errno;

			if (socketError == EINTR)
			{
				/** The blocking function was interrupted by a signal. */

				/** Try again. */
				lockError.unlock();
				continue;
			}
			/* else evaluate using the normal error flow. */

			if ( (socketError == ECONNRESET) ||
				 (socketError == EWOULDBLOCK) )
			{
				/** An incoming connection was indicated, but was subsequently 
					terminated by the remote peer prior to accepting the call. */

				SOCKETLOGD(i->logHandle,KLogTypeGeneral,
					"%s:%d,(0x%08x) benign accept error, socketReturn %d socketError %d", 
					__FUNCTION__,__LINE__,s, socketReturn, socketError);

				/* Continue the while loop. */
				lockError.unlock();
				continue;
			}
			else
			{
				/** Some other socket error occurred, so it is most likely
					that the socket isn't valid, either because of some
					network error or because the user has closed the socket. */
				SOCKETLOGW(i->logHandle,KLogTypeGeneral,
					"%s:%u, terminating accept error, socketReturn %d socketError %d", 
					__FUNCTION__,__LINE__,socketReturn, socketError);

				lockError.unlock();
				break;
			}
		}

		/** An incoming connection is detected, get information about the host. */
		returnValue = _pal_socketSockAddressToHostName(i, incomingAddr.ss_family, &incomingAddr, incomingIP, _ECRIO_SOCKET_IP_SIZE, &incomingPort);
		if (returnValue != KPALErrorNone)
		{
			/* An error occurred, so clean up and continue listening. */

			SOCKETLOGE(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) ec_convertSockAddressToHostName() error, returnValue %d",
				__FUNCTION__,__LINE__,s, returnValue);

			socketReturn = close(IncomingSocket);
			/* Ignore the return value. */

			/* @todo Is there any reason we need to inform this to the Calling Component? */

			/* Continue the while loop. */
			continue;
		}

		/* Remember the IP version of the incoming connection. */
		if (incomingAddr.ss_family == AF_INET6)
		{
			incomingIPVersion = ProtocolVersion_IPv6;
		}
		else
		{
			incomingIPVersion = ProtocolVersion_IPv4;
		}

		/** Create the Socket instance in case the Calling Component wants to
			run with it. */

		/** Allocate memory for this Socket. */
//		pal_MemoryAllocate(sizeof(_EcrioSocketStruct), (void **)&hIncoming);
		hIncoming = new(_EcrioSocketStruct);
		if (hIncoming == nullptr)
		{
			/* Memory for the Socket is not available, so clean up and continue
			   listening. */

			SOCKETLOGE(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) DRVMemoryAllocate() error, returnValue %d", 
				__FUNCTION__,__LINE__,s, returnValue);

			socketReturn = close(IncomingSocket);
			/* Ignore the return value. */

			/* @todo Is there any reason we need to inform this to the Calling Component? */

			/* Continue the while loop. */
			continue;
		}

		/* Set memory of the Socket to 0 to initialize it. */
		pal_MemorySet(hIncoming, 0, sizeof(_EcrioSocketStruct));
		/* Ignoring the return value. */

		/* Initialize the state attributes. */
		hIncoming->bClosed = Enum_FALSE;

		/* Set the socket receive buffer and allocate it. Our convention is to
		   give the buffer size of the TCP Server to the TCP Clients it
		   accepts. */
		hIncoming->bufferSize = s->bufferSize;
		hIncoming->pRecvBuffer = NULL;

		hIncoming->pRecvBuffer = (char *)malloc(s->bufferSize + 1);
		if (hIncoming->pRecvBuffer == NULL)
		{
			/* Memory for the receive buffer is not available. */

			SOCKETLOGE(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) DRVMemoryAllocate() error, returnValue %d",
				__FUNCTION__,__LINE__, s, returnValue);

			socketReturn = close(IncomingSocket);
			/* Ignore the return value. */

			/* Free the memory allocated for the accept socket handle. */
//			pal_MemoryFree((void **)&hIncoming);
			delete hIncoming;
			/* Ignoring the return value. */

			/* @todo Is there any reason we need to inform this to the Calling Component? */

			/* Continue the while loop. */
			continue;
		}

		/* The protocol type will be a TCP client, but already connected
		   of course. */
		hIncoming->protocolType = ProtocolType_TCP_Client;

		/* The protocol version we be the same as that of the TCP server. */
		hIncoming->protocolVersion = incomingIPVersion;

		hIncoming->socketType = SocketType_Default;
		hIncoming->socketMediaType = SocketMediaType_Default;

		hIncoming->syncSendFlags = 0;

		/* Save the socket reference. */
		hIncoming->socket = IncomingSocket;

		/* @todo Note that we aren't configuring the socket in any way. Are
		   we allowed to call setsockopt() at this point? Do we need to? */

		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) potential accept hIncoming 0x%08x incomingIP %s incomingPort %d",
			__FUNCTION__,__LINE__, s, hIncoming, &incomingIP[0], incomingPort);

		/* Initialize the accept flag for the next use. */
		bAccept = Enum_FALSE;

		/** Invoke the Calling Component's accept callback to get permission
			to accept the incoming connection or not. Make sure the TCP Server
			hasn't been closed first. Note that in the current design, the
			callback can't (shouldn't) invoke any Socket functions for the
			newly accepted client. One option will be to invoke the callback
			again with bAccept = Enum_TRUE indicating that everything has been
			setup. Or else we can just wait for incoming data. But what
			happens between the time the incoming socket is detected until
			we are done with the setup? */
		if (!s->bClosed)
		{
			if (s->bEnableGlobalMutex)
			{
				/* Global mutex locked. */
				//DRVGlobalMutexLock();
				pal_MutexGlobalLock(i->pal);
			}

			s->callback.pAcceptCallbackFn(
				s,
				hIncoming,
				s->callback.pCallbackFnData,
				incomingIPVersion,
				(const u_char *)&incomingIP[0],
				incomingPort,
				&bAccept,
				KPALErrorNone);

			if (s->bEnableGlobalMutex)
			{
				/* Global mutex unlocked. */
				//DRVGlobalMutexUnlock();
				pal_MutexGlobalUnlock(i->pal);
			}
		}

		if (bAccept == Enum_FALSE)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) not accepting handle 0x%08x", 
				__FUNCTION__,__LINE__,s, hIncoming);

			/** If the Calling Component does not want to accept the incoming
				connection, then close it and continue to wait for the next
				connection. */
			socketReturn = close(IncomingSocket);
			/* Ignore the return value. */

			/* Free the memory allocated for the accept socket handle. */
			pal_MemoryFree((void **)&hIncoming->pRecvBuffer);
//			pal_MemoryFree((void **)&hIncoming);
			delete hIncoming;
			/* Ignoring the return value. */

			hIncoming->pRecvBuffer = NULL;
			hIncoming = NULL;

			continue;
		}
		else
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) accepting handle 0x%08x", 
				__FUNCTION__,__LINE__,s, hIncoming);

			/* Continue setting up the Socket. */
			bError = false;

			/* Remember the module that the Socket is part of. */
			hIncoming->instance = (SOCKETINSTANCE)i;

			/** Copy the callback data from the TCP Server Socket to the
				incoming Socket. By convention, when the TCP Server was
				setup, it was given the send and receive callback functions
				that all connected clients should use. */
			hIncoming->callback.pReceiveCallbackFn = s->callback.pReceiveCallbackFn;
			hIncoming->callback.pErrorCallbackFn = s->callback.pErrorCallbackFn;
			hIncoming->callback.pAcceptCallbackFn = NULL;
			hIncoming->callback.pCallbackFnData = s->callback.pCallbackFnData;

			hIncoming->bEnableGlobalMutex = s->bEnableGlobalMutex;

			if (!bError)
			{
				if (hIncoming->callback.pReceiveCallbackFn != NULL)
				{
					/** Create the Socket Receive thread only if there is a receive
						callback specified. */
					res = pthread_create(&(hIncoming->hRecvThread), NULL, _pal_SocketRecvWorker, (void *)hIncoming);
					if (res != 0)
					{
						SOCKETLOGE(i->logHandle,KLogTypeGeneral,
							"%s:%u,(0x%08x) pthread_create(receive thread) for 0x%08x failed", 
							__FUNCTION__,__LINE__,s, hIncoming);
						bError = true;
					}
				}
				else
				{
					/* Ensure that hIncoming->hRecvThread is NULL. */
					hIncoming->hRecvThread = 0;
				}

				/** The new Socket is now initialized. Add it to the module list. */
				unique_lock<mutex> lock(i->socketList.m);
				i->socketList.l.push_back(hIncoming);
				lock.unlock();
			}

			if (bError)
			{
				/* An error was detected, so clean up all common resources and
				   continue the overall listening loop. */

				/* We need to lock the Global Mutex so that the ec_SocketClose
				   function can unlock it when waiting. */
				if (s->bEnableGlobalMutex)
				{
					/* Global mutex locked. */
					//DRVGlobalMutexLock();
					pal_MutexGlobalLock(i->pal);
				}

				_pal_SocketClose(i, hIncoming);
				hIncoming = NULL;

				if (s->bEnableGlobalMutex)
				{
					/* Global mutex locked. */
					//DRVGlobalMutexUnlock();
					pal_MutexGlobalUnlock(i->pal);
				}

				/* @todo Signal the Module Error thread to post an error. */

				continue;
			}
		}
	}

	pthread_exit(NULL);
	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u)",__FUNCTION__,__LINE__ );
	/* Should never get here. */
	return NULL;
}

/* Prototypes */

/**
 * Implementation of the pal_socketInit() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_socketInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	SOCKETINSTANCE *instance
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	unsigned int returnValue = KPALErrorNone;
	int res = 0;

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}

	*instance = NULL;

	/** Allocate memory for this instance of the Socket Module. */
//	pal_MemoryAllocate(sizeof(_EcrioSocketInstanceStruct), (void **)&i);
	i = new (_EcrioSocketInstanceStruct);
	if (i == nullptr)
	{
		/* Memory for the Socket Module is not available. */
		return KPALMemoryError;
	}

	SOCKETLOGI(logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	/* Initialize structure attributes. */
	i->pal = pal;
	i->logHandle = logHandle;

	i->uNetID = 0L;

#ifdef ENABLE_TLS_OPENSSL
	/** TLS/SSL startup */
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
#endif /* ENABLE_TLS_OPENSSL */

	i->hSocket = _pal_SocketSignalInit(_pal_SocketHandler, i->logHandle, &returnValue);
	if (i->hSocket == NULL)
	{
		returnValue = KPALInternalError;
		goto Error;
	}

	*instance = (SOCKETINSTANCE)i;

	goto Done;

Error:

//	if (i != NULL)
//	{
//		pal_MemoryFree((void **)&i);
//	}
	delete i;
Done:

	SOCKETLOGI(logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the DRVSocketDeinit() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
void pal_socketDeinit
(
	SOCKETINSTANCE instance
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)instance;

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	while (_pal_socketGetSocket(i, &s) == 0)
	{
		_pal_SocketClose(i, s);
	}

	_pal_SocketSignalDeinit(&i->hSocket);

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);

	/** Free the memory allocated for the Socket Module instance. */
//	pal_MemoryFree((void **)&i);
	delete i;
	/* Ignoring the return value. */
}

/**
 * Implementation of the DRVSocketCreate() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketCreate
(
	PALINSTANCE pal,
	SocketCreateConfigStruct *pConfig,
	SocketCallbackFnStruct *pFnStruct,
	SOCKETHANDLE *handle
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;

	SocketTLSSettingsStruct *pTLSSetting = NULL;

	int addressfamily = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;

	/* On Linux we require the PAL handle to be valid. */
	if (pal == NULL)
	{
		return KPALInvalidHandle;
	}

	/* The pointer to the SocketCreateConfigStruct must not be NULL. */
	if (pConfig == NULL)
	{
		return KPALInvalidParameters;
	}

	/* The pointer to the SocketCallbackFnStruct must not be NULL. */
	if (pFnStruct == NULL)
	{
		return KPALInvalidParameters;
	}

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)((EcrioPALStruct *)pal)->socket;

	/* The pointer to the Instance must not be NULL. */
	if (i == NULL)
	{
		return KPALInvalidHandle;
	}

	*handle = NULL;

	SOCKETLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	SOCKETLOGI(i->logHandle, KLogTypeGeneral, "%s:%u (protocolVersion=%d, protocolType=%d, socketType=%d, socketMediaType=%d, bufferSize=%d, bEnableGlobalMutex=%d"
		, __FUNCTION__, __LINE__, pConfig->protocolVersion, pConfig->protocolType, pConfig->socketType, pConfig->socketMediaType, pConfig->bufferSize, pConfig->bEnableGlobalMutex);

#ifndef ENABLE_TLS_OPENSSL
	if (pConfig->protocolType == ProtocolType_TLS_Client)
	{
		SOCKETLOGE(i->logHandle, KLogTypeGeneral,
			"%s:%u, No TLS library ",
			__FUNCTION__, __LINE__);

		return  KPALNotSupported;
	}
#endif /* ENABLE_TLS_OPENSSL */

	if (pConfig->protocolType == ProtocolType_TLS_Client)
	{

		if (pConfig->tlsSetting == NULL)
		{
			SOCKETLOGE(i->logHandle, KLogTypeGeneral,
				"%s:%u, Wrong input , for TLS client tlsSetting MUST not NULL !",
				__FUNCTION__, __LINE__);

			return KPALInternalError;
		}
		pTLSSetting = pConfig->tlsSetting;
	}

//	pal_MemoryAllocate(sizeof(_EcrioSocketStruct), (void **)&s);
	s = new(_EcrioSocketStruct);
	if (s == nullptr)
	{
		/* Memory for the Socket is not available. */
		return KPALMemoryError;
	}

	/* Set memory of the Socket to 0 to initialize it. */
	pal_MemorySet(s, 0, sizeof(_EcrioSocketStruct));
	/* Ignoring the return value. */

	/* Initialize the state attributes. */
	s->instance = (SOCKETINSTANCE)i;

	/* Saving the information of socket type */
	s->protocolType = pConfig->protocolType;
	s->protocolVersion = pConfig->protocolVersion;
	s->socketType = pConfig->socketType;
	s->socketMediaType = pConfig->socketMediaType;

	s->bEnableGlobalMutex = pConfig->bEnableGlobalMutex;

	/* Set the socket receive buffer and allocate it. */
	s->bufferSize = _ECRIO_SOCKET_RECEIVE_BUFFER_SIZE;
	s->pRecvBuffer = NULL;

	/** Copy the callback data to the internal structure. We don't care if
	    this function has already been called before the Socket is opened. */
	s->callback.pReceiveCallbackFn = pFnStruct->pReceiveCallbackFn;
	s->callback.pErrorCallbackFn = pFnStruct->pErrorCallbackFn;
	s->callback.pAcceptCallbackFn = pFnStruct->pAcceptCallbackFn;
	s->callback.pCallbackFnData = pFnStruct->pCallbackFnData;

	s->localSize = 0;
	s->remoteSize = 0;

	s->bClosed = Enum_FALSE;

	s->syncSendFlags = 0;

	s->hAcceptThread = 0;
	s->hRecvThread = 0;

	switch(s->protocolVersion)
	{
		case ProtocolVersion_IPv6:
		case ProtocolVersion_IPv6_DualStack:
		{
			addressfamily = AF_INET6;
		}
		break;

		case ProtocolVersion_IPv4:
		default:
		{
			/* The Internet Protocol version 4 (IPv4) address family is default setting */
			addressfamily = AF_INET;
		}
		break;
	}

	switch(s->protocolType)
	{
		case ProtocolType_TCP_Client:
		case ProtocolType_TCP_Server:
		case ProtocolType_TLS_Client:
		{
			type = SOCK_STREAM;
			protocol = IPPROTO_TCP;
		}
		break;

		case ProtocolType_UDP:
		default:
		{
			type = SOCK_DGRAM;
			protocol = IPPROTO_UDP;
		}
		break;
	}

	s->uNumOfFingerPrint = 0;
	s->ppFingerPrint = NULL;

	/** Create a socket by calling the socket() Winsock function. */
	s->socket = _ECRIO_SOCKET_ERROR;
	s->socket = socket(addressfamily, type, protocol);
	if (s->socket == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u, socket() failed, socketReturn %d socketError %d",
			__FUNCTION__,__LINE__, s->socket, errno);

//		pal_MemoryFree((void **)&s);
		delete s;
		return  KPALInternalError;
	}

	if (pTLSSetting != NULL && (s->protocolType == ProtocolType_TLS_Client))
	{
		returnValue = _pal_SocketTlsInit(i, s, pTLSSetting);
		if (returnValue != KPALErrorNone)
		{
			SOCKETLOGE(i->logHandle, KLogTypeGeneral,
				"%s:%u, TLS initialize failed",
				__FUNCTION__, __LINE__);

//			pal_MemoryFree((void **)&s);
			delete s;
			return returnValue;
		}
	}

	unique_lock<mutex> lock(i->socketList.m);
	i->socketList.l.push_back(s);
	lock.unlock();

	*handle = (SOCKETHANDLE)s;

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);

	return returnValue;
}

/**
 * Implementation of the DRVSocketSetLocalHost() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketSetLocalHost
(
	SOCKETHANDLE handle,
	const u_char* pLocalIP,
	u_int16 localPort
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;
	unsigned int ipv6Scope = 0;
	struct sockaddr_in6 *local = NULL;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)handle;

	lock_guard<mutex> lock(s->m);

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	if (s->socket == _ECRIO_SOCKET_ERROR)
	{
		return KPALInternalError;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u\tpLocalIP=%s, localPort=%u", __FUNCTION__,__LINE__, pLocalIP, localPort);

	/** Set the socket address information for this host. We don't care if the
		host has already been set or not (before the Socket has been open). 
		The IP String can be point to a valid IP address. Or, if it is NULL
		or set to an "INADDR_ANY" or in6addr_any, the bind mechanism should
		choose the IP address. */
	returnValue = _pal_SocketSetHostInfo(s, (const char *)pLocalIP, localPort, Enum_TRUE, &s->local, &s->localSize);
	if (returnValue != KPALErrorNone)
	{
		/* The IP string was probably invalid. */
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u, (0x%08x) ec_setHostInfo() failed",
			__FUNCTION__,__LINE__, s);

		goto Error;
	}

	if (s->protocolVersion != ProtocolVersion_IPv4)
	{
		if (pal_StringLength((const u_char *)i->deviceName) != 0)
		{
			ipv6Scope = if_nametoindex(i->deviceName);
			local = (struct sockaddr_in6 *)&(s->local);
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tlocal->sin6_scope_id=%u, ipv6Scope=%u, ifname=%s",__FUNCTION__,__LINE__, local->sin6_scope_id, ipv6Scope, i->deviceName);
			if (local->sin6_scope_id != ipv6Scope)
			{
				local->sin6_scope_id = ipv6Scope;
			}
		}
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return KPALErrorNone;

Error:

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the DRVSocketSetRemoteHost() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketSetRemoteHost
(
	SOCKETHANDLE handle,
	const u_char* pRemoteIP,
	u_int16 remotePort
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;

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

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)handle;

	lock_guard<mutex> lock(s->m);

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u\tpRemoteIP=%s, remotePort=%u", __FUNCTION__,__LINE__, pRemoteIP, remotePort);

	/** If the Socket is open, we can't change the remote host for TCP. The
		Calling Component is expected to close and re-create the Socket. */
	if ((s->protocolType != ProtocolType_UDP) && (s->hRecvThread != 0))
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u, (0x%08x) Socket is already open", 
			__FUNCTION__,__LINE__,s);
		returnValue = KPALAlreadyInUse;
		goto Error;
	}

	if ( (s->protocolType == ProtocolType_UDP) ||
		 (s->protocolType == ProtocolType_TCP_Client) ||
		 (s->protocolType == ProtocolType_TLS_Client))
	{
		/** Set the socket address information for this host. We don't care if the
			host has already been set or not (before the Socket has been open). */
		returnValue = _pal_SocketSetHostInfo(s, (const char *)pRemoteIP, remotePort, Enum_FALSE, &s->remote, &s->remoteSize);
		if (returnValue != KPALErrorNone)
		{
			/* The IP string was probably invalid. */
			SOCKETLOGE(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) ec_setHostInfo() failed", 
				__FUNCTION__,__LINE__,s);

			goto Error;
		}

		if (remotePort == 0)
		{
			if ((s->protocolType == ProtocolType_TCP_Client) ||
				(s->protocolType == ProtocolType_TLS_Client))
			{
				/* Remote port can't be 0 for TCP clients. There is no recovery
				   to a prior state, so ensure that no other functions can
				   succeed. However, this function can be called again. */
				returnValue = KPALNotSupported;
				goto Error;
			}
		}

		/** For UDP, we will re-establish the connection if the connection is
			open. */
		if ((s->protocolType == ProtocolType_UDP) && (s->hRecvThread != 0))
		{
			/** Associate the configured remote host information with the
				socket by calling the connect() API. This is valid for UDP
				protocols because it will establish the default destination
				address for subsequent calls to send/recv. */
			do
			{
				socketReturn = connect(s->socket, (struct sockaddr *)&s->remote, s->remoteSize);
				/** If there is an EINTR error, we assume for Linux that the
					connect call can be invoked again, the so called "Liberal
					Behavior" as per http://www.madore.org/~david/computers/connect-intr.html. */
			} while ( (socketReturn == _ECRIO_SOCKET_ERROR) && (errno == EINTR) );

			if (socketReturn == _ECRIO_SOCKET_ERROR)
			{
				SOCKETLOGE(i->logHandle,KLogTypeGeneral,
					"%s:%u,(0x%08x) connect() failed, socketReturn %d socketError %d", 
					__FUNCTION__,__LINE__,s, socketReturn, errno);

				returnValue = KPALInternalError;
				goto Error;
			}
		}
	}
	else if (s->protocolType == ProtocolType_TCP_Server)
	{
		/* Setting a remote host for a TCP Server is not allowed. The current
		   interface does not provide any security mechanism for limiting
		   incoming connections. This is up the Calling Component. */
		returnValue = KPALNotSupported;
		goto Error;
	}
	else
	{
		/* Nothing is changed so no need to reset flags. */
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u, (0x%08x) Invalid protocol", 
			__FUNCTION__,__LINE__,s);
		returnValue = KPALInsufficientParameters;
		goto Error;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return KPALErrorNone;

Error:

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the DRVSocketGetLocalHost() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketGetLocalHost
(
	SOCKETHANDLE handle,
	u_char* pLocalIP,
	u_int16* pLocalPort
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	struct sockaddr_storage Addr = {0};
	int AddrLen = 0;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;
	const struct sockaddr_in *sa_in = NULL;
	const void *addr = NULL;

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

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)handle;

	lock_guard<mutex> lock(s->m);

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	if (s->localSize == 0)
	{
		return KPALEntityInvalid;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	/** The socket is currently open. Use getsockname() to find out
		the actual socket details in case the operating system had to
		choose the IP and/or port. */
	AddrLen = sizeof(Addr);

	socketReturn = getsockname(s->socket, (struct sockaddr *)&Addr, (socklen_t *)&AddrLen);
	if (socketReturn == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u, (0x%08x) getsockname() failed, socketReturn %d socketError %d", 
			__FUNCTION__,__LINE__,s, socketReturn, errno);
		returnValue = KPALInternalError;
		goto Error;
	}

	/** Get the IP and port from the sockaddr_storage structure, so it will
		be read as either IPv4 or IPv6 by using getnameinfo(). */

	sa_in = (const struct sockaddr_in *)(struct sockaddr *)&Addr;
	addr = &sa_in->sin_addr;
	inet_ntop(AF_INET, addr, (char *)pLocalIP, _ECRIO_SOCKET_IP_SIZE);

	*pLocalPort = ntohs(((struct sockaddr_in*)(&Addr))->sin_port);

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u\tpLocalIP=%s, LocalPort=%u",__FUNCTION__,__LINE__, pLocalIP, *pLocalPort);
	return KPALErrorNone;

Error:

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the DRVSocketGetRemoteHost() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketGetRemoteHost
(
	SOCKETHANDLE handle,
	u_char* pRemoteIp,
	u_int16* pRemotePort
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	struct sockaddr_storage Addr = {0};
	int AddrLen = 0;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;

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

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)handle;

	lock_guard<mutex> lock(s->m);

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	if (s->remoteSize == 0)
	{
		return KPALEntityInvalid;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	if ( (s->protocolType == ProtocolType_UDP) ||
		 (s->protocolType == ProtocolType_TCP_Client) ||
		 (s->protocolType == ProtocolType_TLS_Client) )
	{
		/** The socket is currently open. Use getpeername to find out
			the actual socket details in case the operating system had
			to choose the IP and/or port. */
		AddrLen = sizeof(Addr);

		socketReturn = getpeername(s->socket, (struct sockaddr *)&Addr, (socklen_t *)&AddrLen);
		if (socketReturn == _ECRIO_SOCKET_ERROR)
		{
			SOCKETLOGE(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) getpeername() failed, socketReturn %d socketError %d", 
				__FUNCTION__,__LINE__,s, socketReturn, errno);
			returnValue = KPALInternalError;
			goto Error;
		}

		/** Get the IP and port from the sockaddr_storage structure, so it will
			be read as either IPv4 or IPv6 by using getnameinfo(). */

		// @note This won't work for IPv6...
		// @note This will only work with addresses provided by IP.
		const struct sockaddr_in *sa_in = (const struct sockaddr_in *)(struct sockaddr *)&Addr;
		const void *addr = &sa_in->sin_addr;
		inet_ntop(AF_INET, addr, (char *)pRemoteIp, _ECRIO_SOCKET_IP_SIZE);

		*pRemotePort = ntohs(((struct sockaddr_in*)(&Addr))->sin_port);
	}
	else if (s->protocolType == ProtocolType_TCP_Server)
	{
		/* Getting a remote host for a TCP Server is not allowed. A TCP Server
		   does not need a remote host. */
		returnValue = KPALNotSupported;
		goto Error;
	}
	else
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) Invalid protocol",
			__FUNCTION__,__LINE__, s);
		returnValue = KPALInsufficientParameters;
		goto Error;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u\tpRemoteIp=%s, RemotePort=%u",__FUNCTION__,__LINE__, pRemoteIp, *pRemotePort);
	return KPALErrorNone;

Error:

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the socket open function when proxy is enabled. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketOpenEx
(
	SOCKETHANDLE handle
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;
	int res = 0;
	
	int flags, oflags;
	int iOptVal;	
	int iOptLen;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)handle;

	lock_guard<mutex> lock(s->m);

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	if (s->socket == _ECRIO_SOCKET_ERROR || s->localSize == 0)
	{
		return KPALNotAllowed;
	}

	if ( ( (s->protocolType == ProtocolType_TCP_Client) || 
		(s->protocolType == ProtocolType_TLS_Client) ) &&
		 (s->remoteSize == 0) )
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) Remote host is not set for TCP Client", 
			__FUNCTION__,__LINE__,s);
			
		returnValue = KPALNotAllowed;
		goto Error;
	}

	/** check if TLS is properly initialized for this socket*/
	if ((s->protocolType == ProtocolType_TLS_Client ) 
		&& s->bTLSInit == Enum_FALSE)
	{
		SOCKETLOGE(i->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) TLS is not initialized for this socket",
			__FUNCTION__, __LINE__, s);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tprotocolVersion=%u, protocolType=%u",__FUNCTION__,__LINE__, s->protocolVersion, s->protocolType);

	iOptVal = 0;
	iOptLen = sizeof(iOptVal);
	res = getsockopt(s->socket, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, (socklen_t*)&iOptLen);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) getsockopt(SO_RCVBUF) failed, res %d socketError %d",
			__FUNCTION__,__LINE__, s, res, errno);
			
		returnValue = KPALInternalError;
		goto Error;
	}
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tget original SO_RCVBUF=%d",__FUNCTION__,__LINE__, iOptVal);

	iOptVal = s->bufferSize;
	iOptLen = sizeof(iOptVal);
	res = setsockopt(s->socket, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, iOptLen);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) setsockopt(SO_RCVBUF) failed, res %d socketError %d",
			__FUNCTION__,__LINE__, s, res, errno);
			
		returnValue = KPALInternalError;
		goto Error;
	}

	iOptVal = 0;
	iOptLen = sizeof(iOptVal);
	res = getsockopt(s->socket, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, (socklen_t*)&iOptLen);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) getsockopt(SO_RCVBUF) failed, res %d socketError %d",
			__FUNCTION__,__LINE__, s, res, errno);
			
		returnValue = KPALInternalError;
		goto Error;
	}
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tget new SO_RCVBUF=%d",__FUNCTION__,__LINE__, iOptVal);

	if (iOptVal > s->bufferSize)
	{
		s->bufferSize = iOptVal;
	}

	/* We don't allocate any receive buffer for TCP Servers. They only accept
	   incoming socket connections and never receive data, at least none that
	   callers need to be informed about. */
	if (s->protocolType != ProtocolType_TCP_Server)
	{
		pal_MemoryAllocate(s->bufferSize + 1, (void **)&s->pRecvBuffer);
		if (s->pRecvBuffer == NULL)
		{
			/* Memory for the receive buffer is not available. */
			returnValue = KPALMemoryError;
			goto Error;
		}
	}

#ifdef ENABLE_QCMAPI
	if (i->uNetID != 0L)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tandroid_setsocknetwork() is calling. uNetID=%llu, h->socket=%u",__FUNCTION__,__LINE__, i->uNetID, s->socket);
		res = android_setsocknetwork((net_handle_t)i->uNetID, s->socket);
		if (res == _ECRIO_SOCKET_ERROR)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) android_setsocknetwork() failed, res %d socketError %d",
				__FUNCTION__,__LINE__, s, res, errno);
				
			returnValue = KPALInternalError;
			goto Error;
		}
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tandroid_setsocknetwork() is returned res=%d",__FUNCTION__,__LINE__, res);
	}
#endif

    if (s->protocolType == ProtocolType_TCP_Server) {
		/** Associate the configured local host information with the socket by
			calling the bind() API. */
		{
			char *pAddr = NULL;
			if (s->protocolVersion == ProtocolVersion_IPv4)
			{
				struct sockaddr_in *addr_in = (struct sockaddr_in *)&s->local;
				pAddr = (char *)malloc(INET_ADDRSTRLEN);
				inet_ntop(AF_INET, &(addr_in->sin_addr), pAddr, INET_ADDRSTRLEN);
			}
			else
			{
				struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&s->local;
				pAddr = (char *)malloc(INET6_ADDRSTRLEN);
				inet_ntop(AF_INET6, &(addr_in6->sin6_addr), pAddr, INET6_ADDRSTRLEN);
			}
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tbind() is calling, local=%s, h->localSize=%u",__FUNCTION__,__LINE__, pAddr, s->localSize);
			free(pAddr);
		}

		res = ::bind(s->socket, (struct sockaddr *)&s->local, s->localSize);
		if (res == _ECRIO_SOCKET_ERROR)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) bind() failed, res %d socketError %d",
				__FUNCTION__,__LINE__, s, res, errno);
				
			returnValue = KPALInternalError;
			goto Error;
		}
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tbind() is returned res=%d",__FUNCTION__,__LINE__, res);
    	}

	if (s->protocolType == ProtocolType_TCP_Server)
	{
		/** Set the socket to listen for incoming connections. */
		res = listen(s->socket, SOMAXCONN);
		if (res == _ECRIO_SOCKET_ERROR)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) listen() failed, res %d socketError %d",
				__FUNCTION__,__LINE__, s, res, errno);
				
			returnValue = KPALInternalError;
			goto Error;
		}

//		s->hAcceptThread = thread(_pal_SocketAcceptWorker, (void *)s);
		/** Create the Socket TCP Accept thread. */
		res = pthread_create(&(s->hAcceptThread), NULL, _pal_SocketAcceptWorker, (void *)s);
		if (res != 0)
		{
			SOCKETLOGE(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) CreateThread(accept thread) failed", 
				__FUNCTION__,__LINE__,s);
				
			returnValue = KPALInternalError;
			goto Error;
		}
	}
	else
	{
		if (s->remoteSize > 0)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_GETFL) is calling",__FUNCTION__,__LINE__);
			res = flags = fcntl(s->socket, F_GETFL, 0);
			if (res == -1)
			{
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, fcntl(F_GETFL) failed, res %d errno %d",
					__FUNCTION__, __LINE__, res, errno);
				returnValue = KPALInternalError;
				goto Error;
			}
			oflags = flags;
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, res);

			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_SETFL) is calling, flags | O_NONBLOCK =%d",__FUNCTION__,__LINE__, flags | O_NONBLOCK);
			res = fcntl(s->socket, F_SETFL, flags | O_NONBLOCK);
			if (res == -1)
			{
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, fcntl(F_SETFL) failed, res %d errno %d",
					__FUNCTION__, __LINE__, res, errno);
				returnValue = KPALInternalError;
				goto Error;
			}
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, res);

			struct timeval tv;
			fd_set wset;
			int valopt;
			socklen_t lon;
			{
				char *pAddr = NULL;
				if (s->protocolVersion == ProtocolVersion_IPv4)
				{
					struct sockaddr_in *addr_in = (struct sockaddr_in *)&s->remote;
					pAddr = (char *)malloc(INET_ADDRSTRLEN);
					inet_ntop(AF_INET, &(addr_in->sin_addr), pAddr, INET_ADDRSTRLEN);
				}
				else
				{
					struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&s->remote;
					pAddr = (char *)malloc(INET6_ADDRSTRLEN);
					inet_ntop(AF_INET6, &(addr_in6->sin6_addr), pAddr, INET6_ADDRSTRLEN);
				}
				SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tconnect() is calling, remote=%s, h->remoteSize=%u",__FUNCTION__,__LINE__, pAddr, s->remoteSize);
				free(pAddr);
			}
			res = connect(s->socket, (struct sockaddr *)&s->remote, s->remoteSize);
			if (res != 0)
			{
				SOCKETLOGI(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) connect() res=%d, errno=%d", __FUNCTION__, __LINE__, s, res, errno);
				if (errno == EINPROGRESS)
				{
					tv.tv_sec = 16;
					tv.tv_usec = 0;
					FD_ZERO(&wset);
					FD_SET(s->socket, &wset);
					res = select(s->socket + 1, NULL, &wset, NULL, &tv);
					if (res < 0)
					{
						returnValue = KPALInternalError;
						SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) connecting is failed res=%", __FUNCTION__, __LINE__, s, res);
						goto Error;
					}

					lon = sizeof(int);
					if (getsockopt(s->socket, SOL_SOCKET, SO_ERROR, (char *)(&valopt), &lon) < 0)
					{
						returnValue = KPALInternalError;
						SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) getsockopt() res=%d", __FUNCTION__, __LINE__, s, res);
						goto Error;
					}

					if (valopt)
					{
						returnValue = KPALInternalError;
						SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) Error in delayed connection socketError=%d", __FUNCTION__, __LINE__, s, valopt);
						goto Error;
					}
				}
				else
				{
					returnValue = KPALInternalError;
					SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) connect() res=%d, socketError=%d", __FUNCTION__, __LINE__, s, res, errno);
					goto Error;
				}
			}
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tconnect() is returned res=%d",__FUNCTION__,__LINE__, res);

			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_SETFL) is calling, oflags =%d",__FUNCTION__,__LINE__, oflags);
			res = fcntl(s->socket, F_SETFL, oflags);
			if (res == -1)
			{
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, fcntl(F_SETFL) failed, res %d socketError %d",
					__FUNCTION__, __LINE__, s->socket, errno);
				returnValue = KPALInternalError;
				goto Error;
			}

			/** For TLS socket, do connection/handshake with TLS server */
			if (s->protocolType == ProtocolType_TLS_Client) 
			{
				/** Connect using TLS */
				returnValue = _pal_SocketTlsConnect(s);

				if (returnValue != KPALErrorNone )
				{
					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) TLS connection failed",
						__FUNCTION__, __LINE__, s);

					goto Error;
				}
				else
				{
					/** Check peer certificate */
					returnValue = _pal_SocketTlsServerCertificate(s);

					if (returnValue != KPALErrorNone)
					{
						SOCKETLOGE(i->logHandle, KLogTypeGeneral,
							"%s:%u,(0x%08x) TLS peer certificate verification failed",
							__FUNCTION__, __LINE__, s);
						goto Error;
					}
				}
			}
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, res);
		}

		if (s->callback.pReceiveCallbackFn != NULL)
		{
//			s->hRecvThread = thread(_pal_SocketRecvWorker, (void *)s);
			res = pthread_create(&(s->hRecvThread), NULL, _pal_SocketRecvWorker, (void *)s);
			if (res != 0)
			{
				SOCKETLOGE(i->logHandle,KLogTypeGeneral,
					"%s:%u,(0x%08x) CreateThread(recv thread) failed", 
					__FUNCTION__,__LINE__,s);
					
				returnValue = KPALInternalError;
				goto Error;
			}
		}
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return KPALErrorNone;

Error:

	/** The Calling Component must call DRVSocketClose() to properly clean up
		all threads and resources in case of error. */
	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the DRVSocketOpen() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketOpen
(
	SOCKETHANDLE handle
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;
	int res = 0;
	
	int flags, oflags;
	int iOptVal;	
	int iOptLen;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)handle;

	lock_guard<mutex> lock(s->m);

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	if (s->socket == _ECRIO_SOCKET_ERROR || s->localSize == 0)
	{
		return KPALNotAllowed;
	}

	if ( ( (s->protocolType == ProtocolType_TCP_Client) || 
		(s->protocolType == ProtocolType_TLS_Client) ) &&
		 (s->remoteSize == 0) )
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) Remote host is not set for TCP Client", 
			__FUNCTION__,__LINE__,s);
			
		returnValue = KPALNotAllowed;
		goto Error;
	}

	/** check if TLS is properly initialized for this socket*/
	if ((s->protocolType == ProtocolType_TLS_Client ) 
		&& s->bTLSInit == Enum_FALSE)
	{
		SOCKETLOGE(i->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) TLS is not initialized for this socket",
			__FUNCTION__, __LINE__, s);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tprotocolVersion=%u, protocolType=%u",__FUNCTION__,__LINE__, s->protocolVersion, s->protocolType);

	iOptVal = 0;
	iOptLen = sizeof(iOptVal);
	res = getsockopt(s->socket, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, (socklen_t*)&iOptLen);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) getsockopt(SO_RCVBUF) failed, res %d socketError %d",
			__FUNCTION__,__LINE__, s, res, errno);
			
		returnValue = KPALInternalError;
		goto Error;
	}
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tget original SO_RCVBUF=%d",__FUNCTION__,__LINE__, iOptVal);

	iOptVal = s->bufferSize;
	iOptLen = sizeof(iOptVal);
	res = setsockopt(s->socket, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, iOptLen);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) setsockopt(SO_RCVBUF) failed, res %d socketError %d",
			__FUNCTION__,__LINE__, s, res, errno);
			
		returnValue = KPALInternalError;
		goto Error;
	}

	iOptVal = 0;
	iOptLen = sizeof(iOptVal);
	res = getsockopt(s->socket, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, (socklen_t*)&iOptLen);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) getsockopt(SO_RCVBUF) failed, res %d socketError %d",
			__FUNCTION__,__LINE__, s, res, errno);
			
		returnValue = KPALInternalError;
		goto Error;
	}
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tget new SO_RCVBUF=%d",__FUNCTION__,__LINE__, iOptVal);

	if (iOptVal > s->bufferSize)
	{
		s->bufferSize = iOptVal;
	}

	/* We don't allocate any receive buffer for TCP Servers. They only accept
	   incoming socket connections and never receive data, at least none that
	   callers need to be informed about. */
	if (s->protocolType != ProtocolType_TCP_Server)
	{
		pal_MemoryAllocate(s->bufferSize + 1, (void **)&s->pRecvBuffer);
		if (s->pRecvBuffer == NULL)
		{
			/* Memory for the receive buffer is not available. */
			returnValue = KPALMemoryError;
			goto Error;
		}
	}

#ifdef ENABLE_QCMAPI
	if (i->uNetID != 0L)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tandroid_setsocknetwork() is calling. uNetID=%llu, h->socket=%u",__FUNCTION__,__LINE__, i->uNetID, s->socket);
		res = android_setsocknetwork((net_handle_t)i->uNetID, s->socket);
		if (res == _ECRIO_SOCKET_ERROR)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) android_setsocknetwork() failed, res %d socketError %d",
				__FUNCTION__,__LINE__, s, res, errno);
				
			returnValue = KPALInternalError;
			goto Error;
		}
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tandroid_setsocknetwork() is returned res=%d",__FUNCTION__,__LINE__, res);
	}
#endif

	/** Associate the configured local host information with the socket by
		calling the bind() API. */
	{
		char *pAddr = NULL;
		if (s->protocolVersion == ProtocolVersion_IPv4)
		{
			struct sockaddr_in *addr_in = (struct sockaddr_in *)&s->local;
			pAddr = (char *)malloc(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(addr_in->sin_addr), pAddr, INET_ADDRSTRLEN);
		}
		else
		{
			struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&s->local;
			pAddr = (char *)malloc(INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(addr_in6->sin6_addr), pAddr, INET6_ADDRSTRLEN);
		}
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tbind() is calling, local=%s, h->localSize=%u",__FUNCTION__,__LINE__, pAddr, s->localSize);
		free(pAddr);
	}

	res = ::bind(s->socket, (struct sockaddr *)&s->local, s->localSize);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) bind() failed, res %d socketError %d",
			__FUNCTION__,__LINE__, s, res, errno);
			
		returnValue = KPALInternalError;
		goto Error;
	}
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tbind() is returned res=%d",__FUNCTION__,__LINE__, res);

	if (s->protocolType == ProtocolType_TCP_Server)
	{
		/** Set the socket to listen for incoming connections. */
		res = listen(s->socket, SOMAXCONN);
		if (res == _ECRIO_SOCKET_ERROR)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) listen() failed, res %d socketError %d",
				__FUNCTION__,__LINE__, s, res, errno);
				
			returnValue = KPALInternalError;
			goto Error;
		}

//		s->hAcceptThread = thread(_pal_SocketAcceptWorker, (void *)s);
		/** Create the Socket TCP Accept thread. */
		res = pthread_create(&(s->hAcceptThread), NULL, _pal_SocketAcceptWorker, (void *)s);
		if (res != 0)
		{
			SOCKETLOGE(i->logHandle,KLogTypeGeneral,
				"%s:%u,(0x%08x) CreateThread(accept thread) failed", 
				__FUNCTION__,__LINE__,s);
				
			returnValue = KPALInternalError;
			goto Error;
		}
	}
	else
	{
		if (s->remoteSize > 0)
		{
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_GETFL) is calling",__FUNCTION__,__LINE__);
			res = flags = fcntl(s->socket, F_GETFL, 0);
			if (res == -1)
			{
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, fcntl(F_GETFL) failed, res %d errno %d",
					__FUNCTION__, __LINE__, res, errno);
				returnValue = KPALInternalError;
				goto Error;
			}
			oflags = flags;
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, res);

			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_SETFL) is calling, flags | O_NONBLOCK =%d",__FUNCTION__,__LINE__, flags | O_NONBLOCK);
			res = fcntl(s->socket, F_SETFL, flags | O_NONBLOCK);
			if (res == -1)
			{
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, fcntl(F_SETFL) failed, res %d errno %d",
					__FUNCTION__, __LINE__, res, errno);
				returnValue = KPALInternalError;
				goto Error;
			}
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, res);

			struct timeval tv;
			fd_set wset;
			int valopt;
			socklen_t lon;
			{
				char *pAddr = NULL;
				if (s->protocolVersion == ProtocolVersion_IPv4)
				{
					struct sockaddr_in *addr_in = (struct sockaddr_in *)&s->remote;
					pAddr = (char *)malloc(INET_ADDRSTRLEN);
					inet_ntop(AF_INET, &(addr_in->sin_addr), pAddr, INET_ADDRSTRLEN);
				}
				else
				{
					struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&s->remote;
					pAddr = (char *)malloc(INET6_ADDRSTRLEN);
					inet_ntop(AF_INET6, &(addr_in6->sin6_addr), pAddr, INET6_ADDRSTRLEN);
				}
				SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tconnect() is calling, remote=%s, h->remoteSize=%u",__FUNCTION__,__LINE__, pAddr, s->remoteSize);
				free(pAddr);
			}
			res = connect(s->socket, (struct sockaddr *)&s->remote, s->remoteSize);
			if (res != 0)
			{
				SOCKETLOGI(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) connect() res=%d, errno=%d", __FUNCTION__, __LINE__, s, res, errno);
				if (errno == EINPROGRESS)
				{
					tv.tv_sec = 16;
					tv.tv_usec = 0;
					FD_ZERO(&wset);
					FD_SET(s->socket, &wset);
					res = select(s->socket + 1, NULL, &wset, NULL, &tv);
					if (res < 0)
					{
						returnValue = KPALInternalError;
						SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) connecting is failed res=%", __FUNCTION__, __LINE__, s, res);
						goto Error;
					}

					lon = sizeof(int);
					if (getsockopt(s->socket, SOL_SOCKET, SO_ERROR, (char *)(&valopt), &lon) < 0)
					{
						returnValue = KPALInternalError;
						SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) getsockopt() res=%d", __FUNCTION__, __LINE__, s, res);
						goto Error;
					}

					if (valopt)
					{
						returnValue = KPALInternalError;
						SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) Error in delayed connection socketError=%d", __FUNCTION__, __LINE__, s, valopt);
						goto Error;
					}
				}
				else
				{
					returnValue = KPALInternalError;
					SOCKETLOGE(i->logHandle, KLogTypeGeneral, "%s:%u,(0x%08x) connect() res=%d, socketError=%d", __FUNCTION__, __LINE__, s, res, errno);
					goto Error;
				}
			}
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tconnect() is returned res=%d",__FUNCTION__,__LINE__, res);

			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl(F_SETFL) is calling, oflags =%d",__FUNCTION__,__LINE__, oflags);
			res = fcntl(s->socket, F_SETFL, oflags);
			if (res == -1)
			{
				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, fcntl(F_SETFL) failed, res %d socketError %d",
					__FUNCTION__, __LINE__, s->socket, errno);
				returnValue = KPALInternalError;
				goto Error;
			}

			/** For TLS socket, do connection/handshake with TLS server */
			if (s->protocolType == ProtocolType_TLS_Client) 
			{
				/** Connect using TLS */
				returnValue = _pal_SocketTlsConnect(s);

				if (returnValue != KPALErrorNone )
				{
					SOCKETLOGE(i->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) TLS connection failed",
						__FUNCTION__, __LINE__, s);

					goto Error;
				}
				else
				{
					/** Check peer certificate */
					returnValue = _pal_SocketTlsServerCertificate(s);

					if (returnValue != KPALErrorNone)
					{
						SOCKETLOGE(i->logHandle, KLogTypeGeneral,
							"%s:%u,(0x%08x) TLS peer certificate verification failed",
							__FUNCTION__, __LINE__, s);
						goto Error;
					}
				}
			}
			SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tfcntl() is returned res=%d",__FUNCTION__,__LINE__, res);
		}

		if (s->callback.pReceiveCallbackFn != NULL)
		{
//			s->hRecvThread = thread(_pal_SocketRecvWorker, (void *)s);
			res = pthread_create(&(s->hRecvThread), NULL, _pal_SocketRecvWorker, (void *)s);
			if (res != 0)
			{
				SOCKETLOGE(i->logHandle,KLogTypeGeneral,
					"%s:%u,(0x%08x) CreateThread(recv thread) failed", 
					__FUNCTION__,__LINE__,s);
					
				returnValue = KPALInternalError;
				goto Error;
			}
		}
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return KPALErrorNone;

Error:

	/** The Calling Component must call DRVSocketClose() to properly clean up
		all threads and resources in case of error. */
	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the DRVSocketSendDataSync() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketSendData
(
	SOCKETHANDLE handle,
	const u_char* pDataBuffer,
	u_int32 dataLength
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;
	int socketError = 0;
	unsigned int bytesSent;
	unsigned int bytesLeft;

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

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)handle;

	lock_guard<mutex> lock(s->m);

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	/* @todo Need to figure out when we can leave the critical section! We shouldn't
	keep it going for the whole send process, correct? Shall we make an assumption
	that the caller can't kill the socket handle while we might block here??? */

	if (s->socket == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) the socket must be open", 
			__FUNCTION__,__LINE__,s);
			
		returnValue = KPALEntityInvalid;
		goto Error;
	}

	/** One can't send data from a TCP Server. Only the TCP Client Socket of
		an accepted TCP connection to a TCP Server can. */
	if (s->protocolType == ProtocolType_TCP_Server)
	{
		SOCKETLOGE(i->logHandle,KLogTypeGeneral,
			"%s:%u,(0x%08x) TCP Server can't send data",
			__FUNCTION__,__LINE__,s);

		returnValue = KPALNotSupported;
		goto Error;
	}

	/** All bytes requested to be sent must be sent unless an error
		is found. We will return success if all bytes are sent in
		one or more calls, otherwise an error, even for UDP. */
	bytesSent = 0;
	bytesLeft = dataLength;
	if (s->protocolType == ProtocolType_TLS_Client)
	{
		/** Write buffer using TLS */
		returnValue = _pal_SocketTlsWrite(s, (const char *)&pDataBuffer[bytesSent], bytesLeft, &socketReturn);


		if (returnValue != KPALErrorNone)
		{
			if (returnValue == KPALDataSendErrorBenign)
			{
				/* These errors are benign and won't bring down the
				socket, so the Calling Component will be informed
				of the error, and it can decide to continue sending
				packets, as they may work, or close the Socket. */

				SOCKETLOGD(i->logHandle, KLogTypeGeneral,
					"%s:%u, benign send error, socketError %d",
					__FUNCTION__, __LINE__, errno);
				goto Error;
			}
			else
			{
				/* The Calling Component should close the socket and
				either give up or try again by creating a new one. */


				SOCKETLOGE(i->logHandle, KLogTypeGeneral,
					"%s:%u, fatal send error, socketError %d",
					__FUNCTION__, __LINE__, errno);
				goto Error;
			}
		}

	}
	else
	{
		if (dataLength == 0)
		{
			/* Send an empty packet. */
			socketReturn = send(s->socket, NULL, 0, s->syncSendFlags);
		}
		else
		{
			while (bytesSent < dataLength)
			{
				/* Send to the remote host already set and connected to. */
				socketReturn = send(s->socket, (const char *)&pDataBuffer[bytesSent], bytesLeft, s->syncSendFlags);
				if (socketReturn == _ECRIO_SOCKET_ERROR)
				{
					/* Conserve the error value. */
					socketError = errno;
		
					if (socketError == EINTR)
					{
						/** The blocking function was interrupted by a signal. */
		
						/** Try again. */
						continue;
					}
					else
					{
						/** Break out of the sending loop. */
						break;
					}
				}
		
				bytesSent += socketReturn;
				bytesLeft -= socketReturn;
		
				if (bytesSent < dataLength)
				{
					SOCKETLOGV(i->logHandle,KLogTypeGeneral,
						"%s:%u,(0x%08x) partial send!", 
						__FUNCTION__,__LINE__,s);
				}
		
				/* @todo It might be prudent to have a counter and limit
				   the number of times we try to send data. If we reach
				   the limit, a benign error could be sent to the Calling
				   Component. */
			}
		}

		if (socketReturn == _ECRIO_SOCKET_ERROR)
		{
			if ( (socketError == EWOULDBLOCK) ||
				 (socketError == EHOSTUNREACH) ||
				 (socketError == EFAULT) ||
				 (socketError == ENOBUFS) ||
				 (socketError == EMSGSIZE) ||
				 (socketError == ETIMEDOUT) ||
				 (socketError == EPIPE) )
			{
				/* These errors are benign and won't bring down the
				   socket, so the Calling Component will be informed
				   of the error, and it can decide to continue sending
				   packets, as they may work, or close the Socket. */
				SOCKETLOGD(i->logHandle,KLogTypeGeneral,
					"%s:%u, benign send error, socketReturn %d socketError %d", 
					__FUNCTION__,__LINE__,socketReturn, socketError);

				returnValue = KPALDataSendErrorBenign;
				goto Error;
			}
			else
			{
				/* The Calling Component should close the socket and
				   either give up or try again by creating a new one. */
				SOCKETLOGE(i->logHandle,KLogTypeGeneral,
					"%s:%u, fatal send error, socketReturn %d socketError %d", 
					__FUNCTION__,__LINE__,socketReturn, socketError);

				returnValue = KPALDataSendErrorFatal;
				goto Error;
			}
		}
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return KPALErrorNone;

Error:

	SOCKETLOGI(i->logHandle,KLogTypeFuncExit,"%s:%u",__FUNCTION__,__LINE__);
	return returnValue;
}

/**
 * Implementation of the DRVSocketClose() function. See the
 * EcrioPFD_Socket.h file for interface definitions.
 */
u_int32 pal_SocketClose
(
	SOCKETHANDLE* handle
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	_EcrioSocketStruct *s = NULL;
	unsigned int returnValue = KPALErrorNone;

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

	/* Set the handle pointer to the proper internal pointer. */
	s = (_EcrioSocketStruct *)*handle;

	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)s->instance;

	/* The Module Handle pointer must not be NULL. */
	if (i == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	unique_lock<mutex> lockList(i->socketList.m);
	i->socketList.l.remove(s);
	lockList.unlock();

	if (s->bClosed)
	{
		return KPALInvalidHandle;
	}

	SOCKETLOGI(i->logHandle,KLogTypeFuncEntry,"%s:%u", __FUNCTION__,__LINE__);

	/** Call the internal function _pal_SocketClose() to actually clean up all
		the resources. */
	returnValue = _pal_SocketClose(i, s);

	/** Set the output parameter to NULL, to force NULL in the Calling
		Component's context. */
	*handle = NULL;

	return returnValue;
}

u_int32 pal_SocketSetNetID
(
	PALINSTANCE pal,
	u_int64 uNetID
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	u_int32 error = KPALErrorNone;
	int res = 0;
	
	/* On Linux we require the PAL handle to be valid. */
	if (pal == NULL)
	{
		return KPALInvalidHandle;
	}
	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)((EcrioPALStruct *)pal)->socket;

	/* The pointer to the Instance must not be NULL. */
	if (i == NULL)
	{
		return KPALInvalidHandle;
	}

	if (uNetID == 0L)
	{
		return KPALInternalError;
	}

	SOCKETLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u\tuNetID=%llu", __FUNCTION__, __LINE__, uNetID);

	i->uNetID = uNetID;

#ifdef ENABLE_QCMAPI
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tandroid_setprocnetwork() is calling. uNetID=%llu",__FUNCTION__,__LINE__, i->uNetID);
	res = android_setprocnetwork((net_handle_t)i->uNetID);
	if (res == _ECRIO_SOCKET_ERROR)
	{
		SOCKETLOGD(i->logHandle,KLogTypeGeneral,
			"%s:%u\tandroid_setprocnetwork() failed, res %d socketError %d",
			__FUNCTION__,__LINE__, res, errno);
	}
	SOCKETLOGD(i->logHandle,KLogTypeGeneral,"%s:%u\tandroid_setprocnetwork() is returned res=%d",__FUNCTION__,__LINE__, res);
#endif

	SOCKETLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

u_int32 pal_SocketSetDeviceName
(
	PALINSTANCE pal,
	const char *pDeviceName
)
{
	_EcrioSocketInstanceStruct *i = NULL;
	u_int32 error = KPALErrorNone;

	/* On Linux we require the PAL handle to be valid. */
	if (pal == NULL)
	{
		return KPALInvalidHandle;
	}
	/* Set the module handle pointer to the proper internal pointer. */
	i = (_EcrioSocketInstanceStruct *)((EcrioPALStruct *)pal)->socket;

	/* The pointer to the Instance must not be NULL. */
	if (i == NULL)
	{
		return KPALInvalidHandle;
	}
	
	SOCKETLOGI(i->logHandle, KLogTypeFuncEntry, "%s:%u\tpDeviceName=%s", __FUNCTION__, __LINE__, pDeviceName);

	if (pal_StringLength((const u_char *)pDeviceName) >= 128)
	{
		return KPALInternalError;
	}

	pal_MemorySet(i->deviceName, 0, 128);
	if (pal_StringNCopy((u_char *)i->deviceName, 128, (const u_char *)pDeviceName, pal_StringLength((const u_char *)pDeviceName)) == NULL)
	{
		return KPALInternalError;
	}

	SOCKETLOGI(i->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}
