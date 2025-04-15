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
 * @file EcrioPAL_Socket.c
 * @brief This is the implementation of the Ecrio PAL's Socket Module.
 *
 * Design and implementation comments are removed until they can be properly
 * updated.
 */

/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)

/* Indicate which libraries this implementation needs. */
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "qwave.lib")

/* Standard includes. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Qos2.h>
#include <process.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef ENABLE_TLS_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#endif /* ENABLE_TLS_OPENSSL */

#include "EcrioPAL.h"
#include "EcrioPAL_Internal.h"

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

/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)
/* Definitions. */
#define MODULE_EVENT_ERROR		(WM_USER + 1)	/**< Message used to indicate that an error was detected to the Module Thread. */
#define MODULE_THREAD_EXIT		(WM_USER + 2)	/**< Message used to indicate that the Module Thread should exit. */
#define SOCKET_EVENT_SEND		(WM_USER + 3)	/**< Message used to indicate that the Socket Send Thread should send a packet. */
#define SOCKET_THREAD_EXIT		(WM_USER + 4)	/**< Message used to indicate that the Socket Send Thread should exit. */

#define SOCKET_IP_SIZE			46			/**< The maximum size of an IPv4/IPv6 IP address, including IPv4 tunneling in IPv6 addresses, plus a null terminator. */
#define TLS_TEMP_BUF_SIZE		512			/**< Temp buffer size */
#define TLS_DEFAULT_CONNECTION_TIMEOUT		5000	/**< in millisec */
#define _ECRIO_SOCKET_TLS_HASH_SHA1					"sha-1"
#define _ECRIO_SOCKET_TLS_HASH_SHA224				"sha-224"
#define _ECRIO_SOCKET_TLS_HASH_SHA256				"sha-256"
#define _ECRIO_SOCKET_TLS_HASH_SHA384				"sha-384"
#define _ECRIO_SOCKET_TLS_HASH_SHA512				"sha-512"
#define _ECRIO_SOCKET_TLS_HASH_MD5					"md5"
#define _ECRIO_SOCKET_TLS_HASH_MD2					"md2"
#define TLS_DEFAULT_CIPHER_SUITES "AES128-SHA256:AES128-SHA:DES-CBC3-SHA:NULL-SHA:NULL-SHA256:RC4-SHA:AES256-SHA" /**< 
							Default set of ciphers - 
									TLS_RSA_WITH_AES_128_CBC_SHA256, 
									TLS_RSA_WITH_AES_128_CBC_SHA, 
									TLS_RSA_WITH_3DES_EDE_CBC_SHA,
									TLS_RSA_WITH_NULL_SHA,
									TLS_RSA_WITH_NULL_SHA256,
									TLS_RSA_WITH_RC4_128_SHA,
									TLS_RSA_WITH_AES_256_CBC_SHA*/


/** @enum EcrioPALModuleStateEnum
 * Enumeration holding the module level initialization state.
 */
typedef enum
{
	ECRIO_PAL_MODULE_STATE_UNINITIALIZED = 0,	/**< The state is uninitialized as the Init() function has not completed. */
	ECRIO_PAL_MODULE_STATE_INITIALIZED,			/**< The state is initialized, since the Init() function was successful. */
	ECRIO_PAL_MODULE_STATE_DEINITIALIZING		/**< The state is deinitializing, since the Deinit() function has been called and is either executing or completed. */
} EcrioPALModuleStateEnum;

/** @struct SocketStruct
 * This is the structure maintained by the Socket Module and represents a
 * Socket instance. Memory is allocated for it when the pal_SocketCreate()
 * function is called and it is what the Socket handle points to. All
 * subsequent interface functions used for the socket receive this pointer
 * as parameter input.
 */
typedef struct tag_SocketStruct
{
	struct tag_SocketStruct *next;	/**< Pointer to the next socket structure. */
	SOCKETINSTANCE module;		/**< Reference pointer to the module that the socket belongs to. */

	ProtocolVersionEnum protocolVersion;
	ProtocolTypeEnum protocolType;
	SocketTypeEnum socketType;
	SocketMediaTypeEnum socketMediaType;

	BoolEnum bEnableGlobalMutex;

	SocketCallbackFnStruct callback;	/**< Callback structure used for storing the callbacks provided by the Calling Component. */

	CRITICAL_SECTION socketCS;		/**< The operational critical section for the socket. */

	bool bCreated;					/**< True if the socket is created. */
	bool bCallbacksSet;				/**< True if the callbacks have been set. */
	bool bLocalHostSet;				/**< True if the local host is set. */
	bool bRemoteHostSet;			/**< True if the remote host is set. */
	bool bRemotePortZero;			/**< True if the remote host's port was set to 0 (zero). */
	bool bOpen;						/**< True if the socket is open. */
	bool bClosed;					/**< True if the user closed the socket (different from bOpen = false). */

	SOCKADDR_STORAGE local;			/**< Storage of the local host information in socket address format (IPv6 compatible). Not used once Socket is open. */
	int localSize;					/**< The actual used size of the local host information. */

	SOCKADDR_STORAGE remote;		/**< Storage of the remote host information in socket address format (IPv6 compatible). Not used once Socket is open. */
	int remoteSize;					/**< The actual used size of the remote host information. */

	SOCKET socket;					/**< The Winsock socket object for this socket instance. */

	HANDLE QoSHandle;
	QOS_FLOWID QoSFlowId;

	HANDLE hAcceptThread;			/**< Handle to the socket accept thread. */
	DWORD acceptThreadId;			/**< The socket accept thread ID. */

	HANDLE hRecvThread;				/**< Handle to the socket receive thread. */
	DWORD recvThreadId;				/**< The socket receive thread ID. */

	unsigned int bufferSize;		/**< The buffer size requested by the caller. */
	char *pRecvBuffer;				/**< Pointer to the socket receive buffer. */

#ifdef ENABLE_TLS_OPENSSL
	SSL_CTX *ctx;					/**< SSL context for TLS type socket */
	SSL *ssl;						/**< SSL pointer for TLS type socket */
#endif /* ENABLE_TLS_OPENSSL */
	bool bVerifyPeer;				/**< TRUE if peer certificate verification is set for TLS type socket.Otherwise FALSE */
	bool bTLSInit;					/**< TRUE if the TLS is properly initialized for TLS type socket.Otherwise FALSE */
	unsigned int uConnTimeoutMilli; /**< The value of connection timeout */
	unsigned int uNumOfFingerPrint;
	u_char **ppFingerPrint;
} SocketStruct;

/** @struct ModuleStruct
 * This is the internal structure maintained by the Socket Module and
 * represents a Socket Module instance. Memory is allocated for it when the
 * pal_SocketInit() is called and it is what the Socket Module instance
 * handle points to. Subsequent calls to the pal_SocketCreate() and
 * pal_SocketDeinit() functions utilize this pointer as parameter input.
 */
typedef struct
{
	SocketStruct *head;				/**< Pointer to the head of the socket list for this instance. */
	SocketStruct *tail;				/**< Pointer to the tail of the socket list for this instance. */
	unsigned int socketListSize;	/**< The size of the socket list, the number of sockets tracked by the module. */

	WSADATA wsaData;				/**< The Winsock return information from a call to the WSAStartup() API. */

	PALINSTANCE pal;
	LOGHANDLE logHandle;			/**< If logging is enabled, provide a place to store the logHandle from the caller. */

	HANDLE hThread;					/**< Handle to the module thread. */
	HANDLE hThreadReady;			/**< Handle to the module thread event to synchronize it with Init() and Deinit() functions. */
	DWORD threadId;					/**< The module thread id. */

	volatile EcrioPALModuleStateEnum initState;	/**< The overall module state. */
	CRITICAL_SECTION initCS;		/**< The initialization critical section, used to synchronize the initialization state of the module. */
	CRITICAL_SECTION operationalCS;	/**< The operational critical section, used to synchronize overall public function execution. */
} SocketInstanceStruct;

/** @struct SendStruct
 * This structure is used a container for data needed when posting a message
 * to the socket send thread. It includes all the information necessary for
 * the socket send thread to send a packet.
 */
typedef struct
{
	const u_char *pDataBuffer;	/**< Pointer to the data buffer provided by the Calling Component. */
	unsigned int dataLength;	/**< The amount of data that the data buffer holds. */
	unsigned int context;		/**< The context value associated with the packet to send, provided by the Calling Component. */
	bool useRemote;				/**< True if the remote host is specified by the remote attribute. This usually means that the sendto() function will be used. */
	SOCKADDR_STORAGE remote;	/**< Storage of the remote host information in socket address format. */
} SocketSendStruct;

/* Prototypes */
static unsigned int ec_socketClose
(
	SocketInstanceStruct *m,
	SocketStruct *h
);

/**
 * This function adds a new socket to the list. No order is maintained in the
 * list so the new item will be added to the end of the list. This is optimized
 * because we maintain a pointer to the end of the list, the tail, so there is
 * no need to iterate through the list to find the end.
 */
static void ec_addSocket
(
	SocketInstanceStruct *module,
	SocketStruct *socket
)
{
	if (module->head == NULL)
	{
		/* This is the first timer to add, set it as the head. */
		module->head = socket;

		/** Note: We don't handle or care about the situation where the head
		    is NULL but the tail isn't. It is not likely to happen since we
		    are in charge of all list management here. */
	}
	else
	{
		/* Add the timer to the end of the list. */
		module->tail->next = socket;
	}

	/* Reset the tail to the new end of the list. */
	module->tail = socket;
}

/**
 * This function removes a socket from the list. If the socket to remove is
 * the first in the list, it is quickly removed. Otherwise, we will iterate
 * through the list until we find a match. We return after any problem, such
 * as the head of the list being NULL (meaning there are no items in the list)
 * or if we could not find a match at all. In either of these cases, we
 * consider it to be a positive situation, since we want to remove the socket
 * and it already is not present in the list. No other error or indication is
 * returned to the caller.
 */
static void ec_removeSocket
(
	SocketInstanceStruct *module,
	SocketStruct *socket
)
{
	SocketStruct *prev;
	SocketStruct *next;

	if (module->head == NULL)
	{
		/* There is no list, so return, nothing to do. */
		/* @todo Log this situation. */
		return;
	}

	/* See if the socket to remove is the first one. */
	if (module->head == socket)
	{
		/* Remove the first socket and return. */
		module->head = module->head->next;
		return;
	}

	prev = module->head;
	next = prev->next;

	/* Loop until the socket is found in the list. */
	while (next != NULL)
	{
		/* If the next pointer matches, break out of the loop. */
		if (next == socket)
		{
			break;
		}

		/* The next pointer doesn't match, so continue through the list. */
		prev = next;
		next = next->next;
	}

	if (next == NULL)
	{
		/* No matching socket was found in the list. */
		/* @todo Log this situation. */
		return;
	}

	/* Take the matched socket out of the list. */
	prev->next = next->next;

	/* Reset the tail of the list if necessary. */
	if (next == module->tail)
	{
		module->tail = prev;
	}
}

/**
 * This function will iterate the module's socket list to find the matching
 * socket handle. true is returned if a match is found else false.
 */
static bool ec_findSocketMatch
(
	SocketInstanceStruct *module,
	SocketStruct *socket
)
{
	SocketStruct *iterator;

	iterator = module->head;

	while (iterator != NULL)
	{
		if (iterator == socket)
		{
			return true;
		}
		else
		{
			iterator = iterator->next;
		}
	}

	return false;
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
unsigned int ec_convertSockAddressToHostName
(
	SocketInstanceStruct *m,
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
	m = m;

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
				SOCKETLOGW(m->logHandle, KLogTypeGeneral,
					"%s:%u, getnameinfo(IPv6) failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, socketReturn, WSAGetLastError());

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
				SOCKETLOGW(m->logHandle, KLogTypeGeneral,
					"%s:%u, getnameinfo(IPv4) failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, socketReturn, WSAGetLastError());

				return KPALInternalError;
			}
		}
		break;
	}

	return error;
}

/**
* This function will select TLS method based on the version.
* Currently specific version is not set, Default is recommended.
*/
const void * ec_tlsMethod
(
	SocketStruct *pSockStruct,
	SocketTLSVersionEnum version
)
{
	SocketInstanceStruct *m = NULL;
	const void *method = NULL;

	if (pSockStruct == NULL)
	{
		return NULL;
	}
	m = (SocketInstanceStruct *)pSockStruct->module;


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
			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
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
unsigned int ec_tlsInitialize
(
	SocketInstanceStruct *module, 
	SocketStruct *socket, 
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
		socket->bVerifyPeer = true;
	}
	else
	{
		socket->bVerifyPeer = false;

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
		pTLSSetting->connectionTimeoutMilli : TLS_DEFAULT_CONNECTION_TIMEOUT;

	/** Use default ciphers if not provided */
	pCiphers = (pTLSSetting->pCiphers != NULL) ?
		(const char*)pTLSSetting->pCiphers : TLS_DEFAULT_CIPHER_SUITES;

#ifdef ENABLE_TLS_OPENSSL
	/** Get SSL method */
	method = (SSL_METHOD*)ec_tlsMethod(socket, pTLSSetting->socketTLSVersion);
	if (method == NULL)
	{
		SOCKETLOGE(module->logHandle, KLogTypeGeneral,
			"%s:%u, Wrong version, method is NOT supported!",
			__FUNCTION__, __LINE__);

		returnValue = KPALTLSInternalError;
		goto Error;
	}

	/** Create a new framework to establish TLS/SSL enabled connections	*/
	ctx = SSL_CTX_new(method);   /* Create new context */
	if (ctx == NULL)
	{
		SOCKETLOGE(module->logHandle, KLogTypeGeneral,
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
		SOCKETLOGE(module->logHandle, KLogTypeGeneral,
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
			SOCKETLOGD(module->logHandle, KLogTypeGeneral, "SSL certificate Error code:[%lu](%s),%s,%u",
				SSLError, ERR_error_string(SSLError, NULL), __FUNCTION__, __LINE__);
		}
		else
		{
			if (SSL_CTX_use_PrivateKey_file(ctx, (const char*)pTLSSetting->pCert, SSL_FILETYPE_PEM) == 1)
			{
				SOCKETLOGD(module->logHandle, KLogTypeGeneral,
					"%s:%u, Successfully loaded private key",
					__FUNCTION__, __LINE__);
			}
		}		
	}

	if (socket->bVerifyPeer == true)
	{
		/** Set default locations for trusted CA certificates*/
		if (!SSL_CTX_load_verify_locations(ctx,
			(const char*)pTLSSetting->pCert,
			NULL))
		{
			SOCKETLOGE(module->logHandle, KLogTypeGeneral,
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
		SOCKETLOGE(module->logHandle, KLogTypeGeneral,
			"%s:%u, Unable to create ssl!",
			__FUNCTION__, __LINE__);
		returnValue = KPALTLSInternalError;
		goto Error;
	}

	/** Prepare SSL object to work in client mode*/
	SSL_set_connect_state(ssl);

	/** SSL is initialized, save context & ssl for future use */
	socket->ctx = ctx;
	socket->ssl = ssl;
	socket->bTLSInit = true;

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
	SOCKETLOGE(module->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	return returnValue;
}

/**
* This function sets socket descriptor to TLS and does connect/handshake with 
* server in non-blocking mode.
*/
unsigned int ec_tlsConnect
(
	SocketStruct *pSockStruct
)
{
	SocketInstanceStruct *m = NULL;
	unsigned int returnValue = KPALErrorNone;
	unsigned int timeoutmillisec = 0;
#ifdef ENABLE_TLS_OPENSSL
	int res = 0;
	bool bDone = false;
	unsigned long flags = 1;
#endif /* ENABLE_TLS_OPENSSL */

	if (pSockStruct == NULL)		
	{
		return KPALInternalError;
	}

	m = (SocketInstanceStruct *)pSockStruct->module;

	/** select timeout */
	timeoutmillisec = pSockStruct->uConnTimeoutMilli;

#ifdef ENABLE_TLS_OPENSSL
	/** Set the socket dexcriptor as the input/output facility for the TLS/SSL (encrypted) side 
	of ssl. If socket is blocking/non-blocking, the ssl will also have blocking/non-blocking behaviour.
	*/
	if (!SSL_set_fd(pSockStruct->ssl, pSockStruct->socket))
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) SSL_set_fd() failed, socketError %d",
			__FUNCTION__, __LINE__, pSockStruct, WSAGetLastError());
		returnValue = KPALTLSInternalError;
		goto Error;
	}

	/** Set the underlying socket to non-blocking mode, otherwise SSL_connect would block */
	ioctlsocket(pSockStruct->socket, FIONBIO, &flags);

	ERR_clear_error();
	while (!bDone && ((res = SSL_connect(pSockStruct->ssl)) <= 0))
	{
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
		err = SSL_get_error(pSockStruct->ssl, res);
		switch (err)
		{
			case SSL_ERROR_NONE:
			{
				bDone = true;
			}
			break;

			case SSL_ERROR_WANT_READ:
			{
				selectreturn = select(0, &fdset, NULL, &errset, &tv);
				if (selectreturn == 0)
				{
					/** Timeout is occurred. Set the error & exit the loop*/
					bDone = true;
					returnValue = KPALTLSConnectTimeout;

					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
						"%s:%u,  select() timeout ! timeout(ms) %u result %d, sslerror %d, socketError %d",
						__FUNCTION__, __LINE__, timeoutmillisec,res, err, WSAGetLastError());
				}
				else if (selectreturn < 0)
				{
					/** A failure in select method. Set the error & exit the loop*/
					bDone = true;
					returnValue = KPALTLSInternalError;

					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
						"%s:%u, select() failed! result %d, sslerror %d, socketError %d",
						__FUNCTION__, __LINE__, res, err, WSAGetLastError());
				}
				else	
				{
					if ( FD_ISSET(pSockStruct->socket, &fdset) == 0 )
					{
						bDone = true;
						returnValue = KPALTLSInternalError;
						
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
							"%s:%u, select() read not set! result %d, sslerror %d, socketError %d",
							__FUNCTION__, __LINE__,res, err, WSAGetLastError());
					}
				}
			}
			break;

			case SSL_ERROR_WANT_WRITE:
			{
				selectreturn = select(0, NULL, &fdset, &errset, &tv);
				if (selectreturn == 0)
				{
					/** Timeout is occurred. Set the error & exit the loop*/
					bDone = true;
					returnValue = KPALTLSConnectTimeout;

					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
						"%s:%u,  select() timeout ! timeout(ms) %u, result %d, sslerror %d, socketError %d" ,
						__FUNCTION__, __LINE__, timeoutmillisec,res, err, WSAGetLastError());
				}
				else if (selectreturn < 0)
				{
					/** A failure in select method. Set the error & exit the loop*/
					bDone = true;
					returnValue = KPALTLSInternalError;

					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
						"%s:%u, select() failed! result %d, sslerror %d, socketError %d",
						__FUNCTION__, __LINE__,res, err, WSAGetLastError());
				}
				else	
				{
					if ( FD_ISSET(pSockStruct->socket, &fdset) == 0 )
					{
						bDone = true;
						returnValue = KPALTLSInternalError;
						
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
							"%s:%u, select() write not set! result %d, sslerror %d, socketError %d",
							__FUNCTION__, __LINE__,res, err, WSAGetLastError());
					}
				}
			}
			break;
			
			default:
			{
				/** Most likely a TLS protocol error; Otherwise system error or socket error */
				bDone = true;
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
					if (pSockStruct->bVerifyPeer == true)
					{
						long res = SSL_get_verify_result(pSockStruct->ssl);

						if (res != X509_V_OK)
						{
							SOCKETLOGE(m->logHandle, KLogTypeGeneral,
								"%s:%u, SSL_get_verify_result failed",
								__FUNCTION__, __LINE__);
							returnValue = KPALTLSPeerCertificateError;
						}
					}
				}				
				
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u, SSL Error! result %d, sslerror %d, liberror %u, socketError %d",
					__FUNCTION__, __LINE__, res, err, liberror, WSAGetLastError());
			}
			break;

		}

		ERR_clear_error();
	}

	/** Set the underlying socket back to blocking mode */
	flags = 0;
	ioctlsocket(pSockStruct->socket, FIONBIO, &flags);

Error:

#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(m->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	return returnValue;

}

/**
* This function will write buffer through TLS in blocking mode.
* 
*/
unsigned int ec_tlsWrite
(
	SocketStruct *pSockStruct,
	const char* buffer,
	int bufsize,
	int* pSSLReturn
)
{
	SocketInstanceStruct *m = NULL;
	unsigned int returnValue = KPALErrorNone;
	int res = 0;

	if ((pSockStruct == NULL) ||
		(pSSLReturn == NULL))
	{
		return KPALInternalError;
	}

	m = (SocketInstanceStruct *)pSockStruct->module;

#ifdef ENABLE_TLS_OPENSSL
	/** SSL write on blocking socket */
	ERR_clear_error();
	res = SSL_write(pSockStruct->ssl, buffer, bufsize);
	if (res <= 0) 
	{
		unsigned long liberror = 0;
		int err = SSL_get_error(pSockStruct->ssl, res);		
		
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

			default:
			{
				/** A failure in SSL method, could be a protocol error, system error or a socket error.
				The OpenSSL error queue contains more information on the error. */
				returnValue = KPALDataSendErrorFatal;
				liberror = ERR_get_error();			

				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u, SSL write result %d, sslerror %d, liberror %u, socketError %d",
					__FUNCTION__, __LINE__, res, err, liberror, WSAGetLastError());
			}
			break;

		}
	}

#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(m->logHandle, KLogTypeGeneral,
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
unsigned int ec_tlsRead
(
	SocketStruct *pSockStruct,
	const char* buffer,
	int bufsize,
	int* pSSLReturn
)
{
	SocketInstanceStruct *m = NULL;
	unsigned int returnValue = KPALErrorNone;
	int res = 0;

	if ( (pSockStruct == NULL) ||
		(pSSLReturn == NULL) )
	{
		return KPALInternalError;
	}

	m = (SocketInstanceStruct *)pSockStruct->module;

#ifdef ENABLE_TLS_OPENSSL
	/** SSL read on blocking socket */
	ERR_clear_error();
	res = SSL_read(pSockStruct->ssl, (void*)buffer, bufsize);
	if (res <= 0) 
	{
		unsigned long liberror = 0;
		int err = SSL_get_error(pSockStruct->ssl, res);		
		
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

			default:
			{
				/** A failure in the SSL library occurred, could be a protocol error, system error or a socket error.
				The OpenSSL error queue contains more information on the error. */
				returnValue = KPALDataReceiveErrorFatal;
				liberror = ERR_get_error();

				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u,SSL read result %d, sslerror %d, liberror %u, socketError %d",
					__FUNCTION__, __LINE__, res, err, liberror, WSAGetLastError());
			}
			break;

		}
	}

#else
	/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
	SOCKETLOGE(m->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	*pSSLReturn = res;
	return returnValue;
}

/**
* This function will check peer certifcate information
* And if peer verification is set, this will verify server certificate if present.
*/
unsigned int ec_tlsServerCertificate
(
	SocketStruct *pSockStruct	
)
{
	SocketInstanceStruct *m = NULL;
	unsigned int returnValue = KPALErrorNone;
#ifdef ENABLE_TLS_OPENSSL
	X509 *cert = NULL;
	const EVP_MD *pHashType = NULL;
#endif /* ENABLE_TLS_OPENSSL */
	unsigned int uFingerPrintLength = 0;
	unsigned char fingerPrint[EVP_MAX_MD_SIZE];
	unsigned char fingerPrintStr[EVP_MAX_MD_SIZE*4];
	unsigned char *pHashValue = NULL;
	BoolEnum bMatchedFingerPrint = Enum_FALSE;

	if (pSockStruct == NULL)
	{
		return KPALInternalError;
	}

	m = (SocketInstanceStruct *)pSockStruct->module;

#ifdef ENABLE_TLS_OPENSSL
	/** Get peer certificates (if available) */
	cert = SSL_get_peer_certificate(pSockStruct->ssl); 
	if (cert != NULL)
	{		
		char buf[TLS_TEMP_BUF_SIZE] = { 0 };
		char *line = NULL;

		line = X509_NAME_oneline(X509_get_subject_name(cert), buf, (TLS_TEMP_BUF_SIZE - 1));
		
		SOCKETLOGD(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) CERTIFICATES SUBJECT %s",
			__FUNCTION__, __LINE__, pSockStruct, line ? buf : "[NONE]");
		
		line = X509_NAME_oneline(X509_get_issuer_name(cert), buf, (TLS_TEMP_BUF_SIZE - 1));
		SOCKETLOGD(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) CERTIFICATES ISSUER %s",
			__FUNCTION__, __LINE__, pSockStruct, line ? buf : "[NONE]");
		
		if (pSockStruct->bVerifyPeer == true) 
		{
			long res = SSL_get_verify_result(pSockStruct->ssl);

			if (res != X509_V_OK )
			{
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
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
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
							"%s:%u, unsupported HASH type",
							__FUNCTION__, __LINE__);
						continue;
					}

					pal_MemorySet(fingerPrint, 0, EVP_MAX_MD_SIZE);
					if (!X509_digest(cert, pHashType, fingerPrint, &uFingerPrintLength))
					{
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
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

					SOCKETLOGD(m->logHandle, KLogTypeGeneral,
						"%s:%u,fingerprint from SDP %s",
						__FUNCTION__, __LINE__, pHashValue);

					SOCKETLOGD(m->logHandle, KLogTypeGeneral,
						"%s:%u,fingerprint from certificates %s",
						__FUNCTION__, __LINE__, fingerPrintStr);

					if (pal_StringNICompare(pHashValue, fingerPrintStr, pal_StringLength(fingerPrintStr)) != 0)
					{
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
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
					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
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
		SOCKETLOGD(m->logHandle, KLogTypeGeneral,
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
	SOCKETLOGE(m->logHandle, KLogTypeGeneral,
		"%s:%u,  No SSL Library is defined !",
		__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */

	return returnValue;
}

/**
 * Sets host information in to a given socket address structure by matching
 * the desired IP and port to a specific interface on the device. Can be used
 * for both IPv4 and IPv6.
 *
 * @param[in] pSockStruct	Handle of Socket.
 * @param[in] pIP			An IP address set.
 * @param[in] port			A port number to set.
 * @param[in] bBind			Indicates if the structure will be used in a bind operation.
 * @param[out] addr			The address structure to set.
 * @param[out] addrLen		The size of the address structure that is used.
 * @return Proper PAL error value.
 */
unsigned int ec_setHostInfo
(
	SocketStruct *pSockStruct,
	const char *pIP,
	unsigned short port,
	bool bBind,
	SOCKADDR_STORAGE *addr,
	int *addrLen
)
{
	SocketInstanceStruct *m;
	char sPort[6] = {0};
	int family = 0;
	int socktype = 0;
	int protocol = IPPROTO_UDP;
	struct addrinfo hints;
	struct addrinfo *pRes = NULL;
	struct addrinfo *pRessave = NULL;
	bool bFound = false;
	int socketReturn = 0;

	if (pSockStruct == NULL)
	{
		return KPALInternalError;
	}

	m = (SocketInstanceStruct *)pSockStruct->module;

	sprintf((char *)sPort, "%u", port);

	/* Set the family variable from SocketStruct. If version is not IPv6 or
	   IPv6 Dual Stack then IPv4 will be set. */
	family = ((pSockStruct->protocolVersion == ProtocolVersion_IPv6) ||
		(pSockStruct->protocolVersion == ProtocolVersion_IPv6_DualStack))
		? AF_INET6 : AF_INET;

	/* Set the socket type. */
	switch (pSockStruct->protocolType)
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

	/* Initialize the hints memory to 0. */
	memset((void *)&hints, 0, sizeof(struct addrinfo));

	/* Setup the hints structure to look for the specific constraints we are
	   interested in. */
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	hints.ai_protocol = protocol;

	/* @todo For the time being, we are allowing names to be resolved, but the
	   Socket interfaces do not allow this and we don't really accomodate it
	   fully with IP field sizes limited to SOCKET_IP_SIZE. */
	// hints.ai_flags = AI_NUMERICHOST;
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
		SOCKETLOGW(m->logHandle, KLogTypeGeneral,
			"%s:%u, getaddrinfo() failed, socketReturn %u socketError %d",
			__FUNCTION__, __LINE__, socketReturn, WSAGetLastError());

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
			if ((family == pRes->ai_family) &&
				(socktype == pRes->ai_socktype) &&
				(protocol == pRes->ai_protocol))
			{
				/* Copy the address to the destination structure. */
				memcpy(addr, pRes->ai_addr, (unsigned int)pRes->ai_addrlen);

				/* Hold on to the actual size (the used part of the
				   sockaddr structure) so that we don't need to do all this
				   work again. */
				*addrLen = (int)pRes->ai_addrlen;

				bFound = true;
				break;
			}
			else
			{
				pRes = pRes->ai_next;
			}
		}

		freeaddrinfo(pRessave);
	}

	if (!bFound)
	{
		/* If family and the socket type does not match with the existing one
		   then this error will be returned. This error can occur only when
		   any one of the family or socket type is missing. */
		return KPALEntityNotFound;
	}

	return KPALErrorNone;
}

/**
 * This function serves as the Module Thread. The primary focus of this
 * function is to handle socket errors and invoke the error callback of
 * the Calling Component if any are detected.
 */
static unsigned int WINAPI SocketModuleThread
(
	LPVOID lpParam
)
{
	SocketInstanceStruct *m;
	SocketStruct *h = NULL;
	MSG msg;
	unsigned int error = KPALErrorNone;
	SocketErrorCallback callback = NULL;
	void *pCallbackData = NULL;
	BOOL bReturn;

	/* The parameter does not need to be checked for NULL because we require
	   the init function to properly set it. */
	m = (SocketInstanceStruct *)lpParam;

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Force the system to create the message queue. */
	bReturn = PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	/* Ignore return value. */

	/* Starting the message loop for this thread, fire the event to let the
	   initialization code know the thread is ready. If the event handle is
	   NULL for some reason, ignore it. */
	if (m->hThreadReady != NULL)
	{
		bReturn = SetEvent(m->hThreadReady);
		/* Ignore return value. */
	}

	/** This function implements a message loop and uses the GetMessage()
	    function to block and wait for messages sent to the thread. */
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		/* Check the msg structure for type of message. */
		switch (msg.message)
		{
			/** The MODULE_EVENT_ERROR message is handled which indicates that
			    a socket error has occurred and should be communicated to the
			    Calling Component via an error callback. */
			case MODULE_EVENT_ERROR:
			{
				h = (SocketStruct *)msg.lParam;
				error = (unsigned int)msg.wParam;

				if (h != NULL)
				{
					/** Enter the operational critical section. */
					EnterCriticalSection(&m->operationalCS);

					if (ec_findSocketMatch(m, h))
					{
						/** The relevant pointers are copied within a critical section
						    so they can be used by the callback without worrying that
						    they will be set to NULL or invalidated by another thread
						    closing the socket. */
						callback = h->callback.pErrorCallbackFn;
						pCallbackData = h->callback.pCallbackFnData;
					}

					/** Leave the operational critical section. */
					LeaveCriticalSection(&m->operationalCS);

					if (callback == NULL)
					{
						/* There is no callback so the callback can't be
						   processed. */
						continue;
					}

					if (h->bEnableGlobalMutex)
					{
						pal_MutexGlobalLock(m->pal);
					}

					if (h->bClosed)
					{
						pal_MutexGlobalUnlock(m->pal);
						break;
					}

					/** Send the error from one of the sockets to the
					    Calling Component. */
					callback(
						h,
						pCallbackData,
						error);

					if (h->bEnableGlobalMutex)
					{
						pal_MutexGlobalUnlock(m->pal);
					}

					callback = NULL;
					pCallbackData = NULL;
				}
			}
			break;

			/** The MODULE_THREAD_EXIT event instructs the thread to exit. */
			case MODULE_THREAD_EXIT:
			{
				_endthreadex(0);
			}
			break;

			default:
			{
			}
			break;
		}
	}

	_endthreadex(0);

	error = KPALErrorNone;
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * This function serves as the Receive Thread for handling socket data
 * received from the network and invoking the caller's receive callback.
 */
static unsigned int WINAPI SocketRecvThread
(
	LPVOID lpParam
)
{
	SocketInstanceStruct *m;
	SocketStruct *h;
	int socketReturn = 0;
	int socketError = 0;
	SOCKADDR_STORAGE from = {0};
	int fromLength;
	char fromIP[SOCKET_IP_SIZE] = {0};
	char *pFromIP = NULL;
	unsigned short fromPort = 0;
	unsigned int returnValue = KPALErrorNone;
	BOOL bReturn;
	SocketReceiveStruct tReceive = {0};

	/* The parameter does not need to be checked for NULL because we require
	   the init function to properly set it. */
	h = (SocketStruct *)lpParam;

	m = (SocketInstanceStruct *)h->module;

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** The thread implements an infinite while loop, in which the socket
	    recv() or recvfrom() function will be invoked and block until data
	    is actually received over the socket. If the socket is closed, the
	    functions will return an error which can be handled to exit the
	    loop and hence the thread. Other errors, are processed as needed. */
	for (;;)
	{
		if (h->protocolType == ProtocolType_TLS_Client)
		{
			unsigned int returnError = KPALErrorNone;
			/** Read buffer using TLS*/
			returnError = ec_tlsRead(h, h->pRecvBuffer, h->bufferSize, &socketReturn );

			if (returnError != KPALErrorNone) 
			{
				if (returnError == KPALDataReceiveErrorBenign)
				{				
					/** Enter the operational critical section. */
					EnterCriticalSection(&h->socketCS);

					SOCKETLOGD(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) benign receive error, socketError %d",
						__FUNCTION__, __LINE__, h, h->protocolType, WSAGetLastError());

					/* Only invoke the callback of the Calling Component if the
					socket hasn't been closed. */
					if (!h->bClosed)
					{
						/* Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);

						if (h->callback.pReceiveCallbackFn != NULL)
						{
							tReceive.handle = h;
							tReceive.pReceiveData = NULL;
							tReceive.receiveLength = 0;
							tReceive.pFromIP = NULL;
							tReceive.fromPort = 0;
							tReceive.uTTL_HL = 0;
							tReceive.result = KPALDataReceiveErrorBenign;

							if (h->bEnableGlobalMutex)
							{
								/* Global mutex locked. */
								pal_MutexGlobalLock(m->pal);
							}

							h->callback.pReceiveCallbackFn(
								h->callback.pCallbackFnData,
								&tReceive);

							if (h->bEnableGlobalMutex)
							{
								/* Global mutex unlocked. */
								pal_MutexGlobalUnlock(m->pal);
							}
						}
					}
					else
					{
						/** Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);
					}
					
					continue;
				}
				else if (returnError == KPALEntityClosed)
				{
					/** The Socket is a TCP Client and the receive function indicated
					that 0 bytes were received. The Calling Component should close
					the Socket and either give up or try again because the
					connection has been closed. UDP packets can be zero length. */

					/** Enter the operational critical section. */
					EnterCriticalSection(&h->socketCS);

					SOCKETLOGW(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) terminating receive error, socketError %d",
						__FUNCTION__, __LINE__, h, h->protocolType, WSAGetLastError());

					/* Only post an error to the Calling Component if the user
					didn't intend to close the socket. */
					if (!h->bClosed)
					{
						/* Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);

						/* Signal the Module Error thread to post an error. */
						bReturn = PostThreadMessage(m->threadId, MODULE_EVENT_ERROR, (WPARAM)KPALEntityClosed, (LPARAM)h);
						/* Ignoring the return value. */
					}
					else
					{
						/** Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);
					}

					/* Exit the while loop. */
					break;
				}
				else 
				{
					/** An error indicating an unrecoverable event or the end of
					the socket was encountered. The Calling Component should
					close the socket and either give up or try again. */

					/** Enter the operational critical section. */
					EnterCriticalSection(&h->socketCS);

					SOCKETLOGW(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) terminating receive error, socketError %d",
						__FUNCTION__, __LINE__, h, h->protocolType, WSAGetLastError());

					/* Only post an error to the Calling Component if the user
					didn't actually intend to close the socket. */
					if (!h->bClosed)
					{
						/* Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);

						/* Signal the Module Error thread to post an error. */
						bReturn = PostThreadMessage(m->threadId, MODULE_EVENT_ERROR, (WPARAM)KPALEntityClosed, (LPARAM)h);
						/* Ignoring the return value. */
					}
					else
					{
						/** Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);
					}

					/* Exit the while loop. */
					break;
				}
			}


			if (h->callback.pReceiveCallbackFn != NULL){
				if ((!h->bRemoteHostSet) ||
					(h->bRemotePortZero))
				{
					/** recvfrom was used, so simplify and conserve the remote IP
					address and port to easily send as parameter to the
					Calling Component. */
					returnValue = ec_convertSockAddressToHostName(m, from.ss_family, &from, (char *)fromIP, SOCKET_IP_SIZE, &fromPort);
					pFromIP = &fromIP[0];
					/* @todo It should be possible to speed this up without doing a
					full conversion every time since usually we will recieve
					bursts of data from the same remote host. */

					SOCKETLOGD(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) receive from pFromIP %s fromPort %d",
						__FUNCTION__, __LINE__, h, h->protocolType, pFromIP, fromPort);
				}
				else
				{
					// quiet				SOCKETLOGD(m->logHandle,KLogTypeGeneral,
					// quiet					"%s:%u,(0x%08x, %u) receive from connected remote",
					// quiet					__FUNCTION__,__LINE__, h, h->protocolType);
				}

				/** Enter the operational critical section. */
				EnterCriticalSection(&h->socketCS);

				/* Only post an error to the Calling Component if the user
				didn't intend to close the socket. */
				if (!h->bClosed)
				{
					/* Leave the operational critical section. */
					LeaveCriticalSection(&h->socketCS);

					if (socketReturn == 0)
					{
						tReceive.handle = h;
						tReceive.pReceiveData = NULL;
						tReceive.receiveLength = 0;
						tReceive.pFromIP = (const unsigned char *)pFromIP;
						tReceive.fromPort = fromPort;
						tReceive.uTTL_HL = 0;
						tReceive.result = KPALErrorNone;

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalLock(m->pal);
						}

						/** If the length of the received data is 0, inform the Calling
						Component but set the buffer pointer to NULL because there
						is no valid buffer to point to! 0 length packets are legal
						for UDP. */
						h->callback.pReceiveCallbackFn(
							h->callback.pCallbackFnData,
							&tReceive);

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalUnlock(m->pal);
						}
					}
					else
					{
						tReceive.handle = h;
						tReceive.pReceiveData = (unsigned char *)h->pRecvBuffer;
						tReceive.receiveLength = socketReturn;
						tReceive.pFromIP = (const unsigned char *)pFromIP;
						tReceive.fromPort = fromPort;
						tReceive.uTTL_HL = 0;
						tReceive.result = KPALErrorNone;

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalLock(m->pal);
						}

						/** If there is valid received data, the normal case, inform
						the Calling Component via the receive callback. */
						h->callback.pReceiveCallbackFn(
							h->callback.pCallbackFnData,
							&tReceive);

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalUnlock(m->pal);
						}
					}
				}
				else
				{
					/** Leave the operational critical section. */
					LeaveCriticalSection(&h->socketCS);
				}
			}


		}
		else
		{
			/** If the Calling Component set the remote host and the remote port
			    is not set to zero, the socket recv() function will be used.
			    Otherwise, the recvfrom() function will be used so that the remote
			    host information can be retrieved. */
			if ((h->bRemoteHostSet) &&
				(!h->bRemotePortZero))
			{
				socketReturn = recv(h->socket, h->pRecvBuffer, h->bufferSize, 0);

				// quiet			SOCKETLOGD(m->logHandle,KLogTypeGeneral,
				// quiet				"%s:%u,(0x%08x, %u) recv, socketReturn %d",
				// quiet				__FUNCTION__,__LINE__,h, h->protocolType, socketReturn);
			}
			else
			{
				fromLength = sizeof(from);
				socketReturn = recvfrom(h->socket, h->pRecvBuffer, h->bufferSize, 0, (struct sockaddr *)&from, (socklen_t *)&fromLength);

				SOCKETLOGD(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x, %u) recvfrom, socketReturn %d",
					__FUNCTION__, __LINE__, h, h->protocolType, socketReturn);
			}

			/** Handle possible errors first, in which case the receive function
			    would return SOCKET_ERROR. */
			if (socketReturn == SOCKET_ERROR)
			{
				/* Conserve the error value. */
				socketError = WSAGetLastError();

				if ((h->protocolType == ProtocolType_UDP) &&
					(socketError == WSAECONNRESET))
				{
					/** If there was a ECONNRESET error it usually means that a
					    send() or sendto() was called for a non-existent peer.
					    For UDP, this is usually for sending functions and not
					    fatal, we will send a benign error message to the Calling
					    Component via the module's error callback. */

					SOCKETLOGD(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) benign send error, socketError %d",
						__FUNCTION__, __LINE__, h, h->protocolType, socketError);

					/* Signal the Module Error thread to post an error. */
					bReturn = PostThreadMessage(m->threadId, MODULE_EVENT_ERROR, (WPARAM)KPALDataSendErrorBenign, (LPARAM)h);
					/* Ignoring the return value. */

					continue;
				}
				else if ((socketError == WSAEWOULDBLOCK) ||
					(socketError == WSAEMSGSIZE) ||
					(socketError == WSAETIMEDOUT))
				{
					/** Treating these errors as benign and allow the Calling
					    Component to decide if the socket should be closed or
					    not. Typically, these errors can be ignored and the
					    higher level protocols will attempt to retry. */

					/** Enter the operational critical section. */
					EnterCriticalSection(&h->socketCS);

					SOCKETLOGD(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) benign receive error, socketError %d",
						__FUNCTION__, __LINE__, h, h->protocolType, socketError);

					/* Only invoke the callback of the Calling Component if the
					   socket hasn't been closed. */
					if (!h->bClosed)
					{
						/* Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);

						if (h->callback.pReceiveCallbackFn != NULL)
						{
							tReceive.handle = h;
							tReceive.pReceiveData = NULL;
							tReceive.receiveLength = 0;
							tReceive.pFromIP = NULL;
							tReceive.fromPort = 0;
							tReceive.uTTL_HL = 0;
							tReceive.result = KPALDataReceiveErrorBenign;

							if (h->bEnableGlobalMutex)
							{
								pal_MutexGlobalLock(m->pal);
							}

							h->callback.pReceiveCallbackFn(
								h->callback.pCallbackFnData,
								&tReceive);

							if (h->bEnableGlobalMutex)
							{
								pal_MutexGlobalUnlock(m->pal);
							}
						}
					}
					else
					{
						/** Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);
					}

					/* Continue the while loop. */
					continue;
				}
				else
				{
					/** An error indicating an unrecoverable event or the end of
					    the socket was encountered. The Calling Component should
					    close the socket and either give up or try again. */

					/** Enter the operational critical section. */
					EnterCriticalSection(&h->socketCS);

					SOCKETLOGW(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) terminating receive error, socketError %d",
						__FUNCTION__, __LINE__, h, h->protocolType, socketError);

					/* Only post an error to the Calling Component if the user
					   didn't actually intend to close the socket. */
					if (!h->bClosed)
					{
						/* Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);

						/* Signal the Module Error thread to post an error. */
						bReturn = PostThreadMessage(m->threadId, MODULE_EVENT_ERROR, (WPARAM)KPALEntityClosed, (LPARAM)h);
						/* Ignoring the return value. */
					}
					else
					{
						/** Leave the operational critical section. */
						LeaveCriticalSection(&h->socketCS);
					}

					/* Exit the while loop. */
					break;
				}
			}
			else if ((socketReturn == 0) &&
				(h->protocolType == ProtocolType_TCP_Client))
			{
				/** The Socket is a TCP Client and the receive function indicated
				    that 0 bytes were received. The Calling Component should close
				    the Socket and either give up or try again because the
				    connection has been closed. UDP packets can be zero length. */

				/** Enter the operational critical section. */
				EnterCriticalSection(&h->socketCS);

				SOCKETLOGW(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x, %u) terminating receive error, socketReturn %d",
					__FUNCTION__, __LINE__, h, h->protocolType, socketReturn);

				/* Only post an error to the Calling Component if the user
				   didn't intend to close the socket. */
				if (!h->bClosed)
				{
					/* Leave the operational critical section. */
					LeaveCriticalSection(&h->socketCS);

					/* Signal the Module Error thread to post an error. */
					bReturn = PostThreadMessage(m->threadId, MODULE_EVENT_ERROR, (WPARAM)KPALEntityClosed, (LPARAM)h);
					/* Ignoring the return value. */
				}
				else
				{
					/** Leave the operational critical section. */
					LeaveCriticalSection(&h->socketCS);
				}

				/* Exit the while loop. */
				break;
			}

			if (h->callback.pReceiveCallbackFn != NULL)
			{
				if ((!h->bRemoteHostSet) ||
					(h->bRemotePortZero))
				{
					/** recvfrom was used, so simplify and conserve the remote IP
					    address and port to easily send as parameter to the
					    Calling Component. */
					returnValue = ec_convertSockAddressToHostName(m, from.ss_family, &from, (char *)fromIP, SOCKET_IP_SIZE, &fromPort);
					pFromIP = &fromIP[0];
					/* @todo It should be possible to speed this up without doing a
					   full conversion every time since usually we will recieve
					   bursts of data from the same remote host. */

					SOCKETLOGD(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x, %u) receive from pFromIP %s fromPort %d",
						__FUNCTION__, __LINE__, h, h->protocolType, pFromIP, fromPort);
				}
				else
				{
					// quiet				SOCKETLOGD(m->logHandle,KLogTypeGeneral,
					// quiet					"%s:%u,(0x%08x, %u) receive from connected remote",
					// quiet					__FUNCTION__,__LINE__, h, h->protocolType);
				}

				/** Enter the operational critical section. */
				EnterCriticalSection(&h->socketCS);

				/* Only post an error to the Calling Component if the user
				   didn't intend to close the socket. */
				if (!h->bClosed)
				{
					/* Leave the operational critical section. */
					LeaveCriticalSection(&h->socketCS);

					if (socketReturn == 0)
					{
						tReceive.handle = h;
						tReceive.pReceiveData = NULL;
						tReceive.receiveLength = 0;
						tReceive.pFromIP = (const unsigned char *)pFromIP;
						tReceive.fromPort = fromPort;
						tReceive.uTTL_HL = 0;
						tReceive.result = KPALErrorNone;

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalLock(m->pal);
						}

						/** If the length of the received data is 0, inform the Calling
						    Component but set the buffer pointer to NULL because there
						    is no valid buffer to point to! 0 length packets are legal
						    for UDP. */
						h->callback.pReceiveCallbackFn(
							h->callback.pCallbackFnData,
							&tReceive);

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalUnlock(m->pal);
						}
					}
					else
					{
						tReceive.handle = h;
						tReceive.pReceiveData = (unsigned char *)h->pRecvBuffer;
						tReceive.receiveLength = socketReturn;
						tReceive.pFromIP = (const unsigned char *)pFromIP;
						tReceive.fromPort = fromPort;
						tReceive.uTTL_HL = 0;
						tReceive.result = KPALErrorNone;

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalLock(m->pal);
						}

						/** If there is valid received data, the normal case, inform
						    the Calling Component via the receive callback. */
						h->callback.pReceiveCallbackFn(
							h->callback.pCallbackFnData,
							&tReceive);

						if (h->bEnableGlobalMutex)
						{
							pal_MutexGlobalUnlock(m->pal);
						}
					}
				}
				else
				{
					/** Leave the operational critical section. */
					LeaveCriticalSection(&h->socketCS);
				}
			}
		}
	}

	_endthreadex(0);

	returnValue = KPALErrorNone;
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__ );

	return returnValue;
}

/**
 * This function serves as the Accept Thread for handling incoming TCP
 * connections while listening for them.
 */
static unsigned int WINAPI SocketAcceptThread
(
	LPVOID lpParam
)
{
	SocketInstanceStruct *m;
	SocketStruct *h;
	int socketReturn = 0;
	int socketError = 0;
	SocketStruct *hIncoming = NULL;
	SOCKET IncomingSocket;
	unsigned int returnValue = KPALErrorNone;
	SOCKADDR_STORAGE incomingAddr = {0};
	int	incomingLength;
	char incomingIP[SOCKET_IP_SIZE] = {0};
	ProtocolVersionEnum incomingIPVersion;
	unsigned short incomingPort = 0;
	bool bError;
	BOOL bReturn;

	/* This variable must be defined with the Ecrio data type. */
	BoolEnum bAccept;

	/* The parameter does not need to be checked for NULL because we require
	   the init function to properly set it. */
	h = (SocketStruct *)lpParam;

	m = (SocketInstanceStruct *)h->module;

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** This thread loops listening for incoming TCP Client connections. When
	    a connection request comes in, it is verified with the Calling
	    Component and accepted as a normal TCP Client Socket. The thread loop
	    will exit when the socket is closed. */
	for (;;)
	{
		incomingLength = sizeof(SOCKADDR_STORAGE);

		/** Block while waiting to accept an incoming connection. */
		IncomingSocket = accept(h->socket, (struct sockaddr *)&incomingAddr, (socklen_t *)&incomingLength);
		if (IncomingSocket == INVALID_SOCKET)
		{
			/* Conserve the error value. */
			socketError = WSAGetLastError();

			if ((socketError == WSAECONNRESET) ||
				(socketError == WSAEWOULDBLOCK))
			{
				/** An incoming connection was indicated, but was subsequently
				    terminated by the remote peer prior to accepting the call. */

				SOCKETLOGD(m->logHandle, KLogTypeGeneral,
					"%s:%d,(0x%08x) benign accept error, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, h, socketReturn, socketError);

				/* Continue the while loop. */
				continue;
			}
			else
			{
				/** Some other socket error occurred, so it is most likely
				    that the socket isn't valid, either because of some
				    network error or because the user has closed the socket. */
				SOCKETLOGW(m->logHandle, KLogTypeGeneral,
					"%s:%u, terminating accept error, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, socketReturn, socketError);

				break;
			}
		}

		/** An incoming connection is detected, get information about the host. */
		returnValue = ec_convertSockAddressToHostName(m, incomingAddr.ss_family, &incomingAddr, incomingIP, SOCKET_IP_SIZE, &incomingPort);
		if (returnValue != KPALErrorNone)
		{
			/* An error occurred, so clean up and continue listening. */

			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) ec_convertSockAddressToHostName() error, returnValue %d",
				__FUNCTION__, __LINE__, h, returnValue);

			socketReturn = closesocket(IncomingSocket);
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
		hIncoming = (SocketStruct *)calloc(sizeof(SocketStruct), 1);
		if (hIncoming == NULL)
		{
			/* Memory for the Socket is not available, so clean up and continue
			   listening. */

			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) calloc() error",
				__FUNCTION__, __LINE__, h);

			socketReturn = closesocket(IncomingSocket);
			/* Ignore the return value. */

			/* @todo Is there any reason we need to inform this to the Calling Component? */

			/* Continue the while loop. */
			continue;
		}

		/* Initialize the state attributes. */
		hIncoming->bCreated = false;
		hIncoming->bCallbacksSet = false;
		hIncoming->bLocalHostSet = false;
		hIncoming->bRemoteHostSet = false;
		hIncoming->bRemotePortZero = false;
		hIncoming->bOpen = false;
		hIncoming->bClosed = false;

		/* Set the socket receive buffer and allocate it. Our convention is to
		   give the buffer size of the TCP Server to the TCP Clients it
		   accepts. */
		hIncoming->bufferSize = h->bufferSize;
		hIncoming->pRecvBuffer = NULL;

		hIncoming->pRecvBuffer = (char *)calloc(h->bufferSize, 1);
		if (hIncoming->pRecvBuffer == NULL)
		{
			/* Memory for the receive buffer is not available. */

			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) calloc() error",
				__FUNCTION__, __LINE__, h);

			socketReturn = closesocket(IncomingSocket);
			/* Ignore the return value. */

			/* Free the memory allocated for the accept socket handle. */
			free(hIncoming);

			/* @todo Is there any reason we need to inform this to the Calling Component? */

			/* Continue the while loop. */
			continue;
		}

		/* The protocol type will be a TCP client, but already connected
		   of course. */
		hIncoming->protocolType = ProtocolType_TCP_Client;

		/* The protocol version we be the same as that of the TCP server. */
		hIncoming->protocolVersion = incomingIPVersion;

		/* Save the socket reference. */
		hIncoming->socket = IncomingSocket;

		/* @todo Note that we aren't configuring the socket in any way. Are
		   we allowed to call setsockopt() at this point? Do we need to? */

		SOCKETLOGD(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) potential accept hIncoming 0x%08x incomingIP %s incomingPort %d",
			__FUNCTION__, __LINE__, h, hIncoming, &incomingIP[0], incomingPort);

		/* Initialize the accept flag for the next use. */
		bAccept = Enum_FALSE;

		/** Enter the operational critical section. */
		EnterCriticalSection(&h->socketCS);

		/** Invoke the Calling Component's accept callback to get permission
		    to accept the incoming connection or not. Make sure the TCP Server
		    hasn't been closed first. Note that in the current design, the
		    callback can't (shouldn't) invoke any Socket functions for the
		    newly accepted client. One option will be to invoke the callback
		    again with bAccept = Enum_TRUE indicating that everything has been
		    setup. Or else we can just wait for incoming data. But what
		    happens between the time the incoming socket is detected until
		    we are done with the setup? */
		if (!h->bClosed)
		{
			/** Leave the operational critical section. */
			LeaveCriticalSection(&h->socketCS);

			if (h->bEnableGlobalMutex)
			{
				pal_MutexGlobalLock(m->pal);
			}

			h->callback.pAcceptCallbackFn(
				h,
				hIncoming,
				h->callback.pCallbackFnData,
				incomingIPVersion,
				(const u_char *)&incomingIP[0],
				incomingPort,
				&bAccept,
				KPALErrorNone);

			if (h->bEnableGlobalMutex)
			{
				pal_MutexGlobalUnlock(m->pal);
			}
		}
		else
		{
			/** Leave the operational critical section. */
			LeaveCriticalSection(&h->socketCS);
		}

		if (bAccept == Enum_FALSE)
		{
			SOCKETLOGD(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) not accepting handle 0x%08x",
				__FUNCTION__, __LINE__, h, hIncoming);

			/** If the Calling Component does not want to accept the incoming
			    connection, then close it and continue to wait for the next
			    connection. */
			socketReturn = closesocket(IncomingSocket);
			/* Ignore the return value. */

			/* Free the memory allocated for the accept socket handle. */
			free(hIncoming);

			hIncoming = NULL;

			continue;
		}
		else
		{
			SOCKETLOGD(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) accepting handle 0x%08x",
				__FUNCTION__, __LINE__, h, hIncoming);

			/* Continue setting up the Socket. */
			bError = false;

			/** Enter the operational critical section for the module. */
			EnterCriticalSection(&m->operationalCS);

			/** The new Socket is now initialized. Add it to the module list. */
			hIncoming->next = NULL;
			ec_addSocket(m, hIncoming);
			m->socketListSize++;

			/* Remember the module that the Socket is part of. */
			hIncoming->module = (SOCKETINSTANCE)m;

			/** Leave the operational critical section for the module. */
			LeaveCriticalSection(&m->operationalCS);

			/** Initialize the socket operational critical section. */
			InitializeCriticalSection(&hIncoming->socketCS);
			/* Critical sections are guaranteed by the OS to be initialized. */

			/* The socket is now created. */
			hIncoming->bCreated = true;

			/** Enter the socket operational critical section for the incoming
			    socket. */
			EnterCriticalSection(&hIncoming->socketCS);

			/** Copy the callback data from the TCP Server Socket to the
			    incoming Socket. By convention, when the TCP Server was
			    setup, it was given the send and receive callback functions
			    that all connected clients should use. */
			hIncoming->callback.pReceiveCallbackFn = h->callback.pReceiveCallbackFn;
			hIncoming->callback.pErrorCallbackFn = h->callback.pErrorCallbackFn;
			hIncoming->callback.pAcceptCallbackFn = NULL;
			hIncoming->callback.pCallbackFnData = h->callback.pCallbackFnData;

			hIncoming->bCallbacksSet = true;

			/* The local host is already set by definition, therefore we just
			   have to toggle the flag for it. Once a Socket is open, we don't
			   use the local address structure, so there is no need to
			   initialize it. */
			hIncoming->bLocalHostSet = true;

			/* The remote host is already set by definition, therefore we just
			   have to toggle the flag for it. Once a Socket is open, we don't
			   use the remote address structure, so there is no need to
			   initialize it. The remote port also will be non-zero by
			   definition, so no need to touch the flag for it. */
			hIncoming->bRemoteHostSet = true;

			hIncoming->bEnableGlobalMutex = h->bEnableGlobalMutex;

			if (!bError)
			{
				if (hIncoming->callback.pReceiveCallbackFn != NULL)
				{
					/** Create the Socket Receive thread only if there is a receive
					    callback specified. */
					hIncoming->hRecvThread = (HANDLE)_beginthreadex(NULL, 0, SocketRecvThread, (void *)hIncoming, FALSE, (unsigned int *)&(hIncoming->recvThreadId));
					if (hIncoming->hRecvThread == NULL)
					{
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
							"%s:%u,(0x%08x) CreateThread(receive thread) for 0x%08x failed",
							__FUNCTION__, __LINE__, h, hIncoming);

						bError = true;
					}
				}
				else
				{
					/* Ensure that hIncoming->hRecvThread is NULL. */
					hIncoming->hRecvThread = NULL;
				}
			}

			if (bError)
			{
				/* An error was detected, so clean up all common resources and
				   continue the overall listening loop. */

				/** Leave the operational critical section. */
				LeaveCriticalSection(&hIncoming->socketCS);

				/* We need to lock the Global Mutex so that the ec_SocketClose
				   function can unlock it when waiting. */
				if (h->bEnableGlobalMutex)
				{
					pal_MutexGlobalLock(m->pal);
				}

				ec_socketClose(m, hIncoming);
				hIncoming = NULL;

				if (h->bEnableGlobalMutex)
				{
					pal_MutexGlobalUnlock(m->pal);
				}

				/* Signal the Module Error thread to post an error. */
				bReturn = PostThreadMessage(m->threadId, MODULE_EVENT_ERROR, (WPARAM)KPALEntityInvalid, (LPARAM)h);
				/* Ignoring the return value. */

				continue;
			}
			else
			{
				/* The socket is now open. */
				hIncoming->bOpen = true;

				/** Leave the operational critical section. */
				LeaveCriticalSection(&hIncoming->socketCS);
			}
		}
	}

	_endthreadex(0);

	returnValue = KPALErrorNone;
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u)", __FUNCTION__, __LINE__ );

	return 0;
}

/**
 * Utility function to close the socket and cleanup the internal socket
 * structures.
 */
static unsigned int ec_socketClose
(
	SocketInstanceStruct *m,
	SocketStruct *h
)
{
	int socketReturn = 0;
	BOOL bReturn;
	DWORD dReturn;
#ifdef ENABLE_TLS_OPENSSL
	SSL *ssl = NULL;
	SSL_CTX *ctx = NULL;
	int sslReturn = 0;
#endif /* ENABLE_TLS_OPENSSL */

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** No need to check parameters since this is wrapped with API functions
	    that already do the checking. */

	/* Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	/* Indicate the the user is closing the socket, to prevent any further
	   callbacks. */
	h->bClosed = true;

	if (h->QoSFlowId != 0)
	{
		bReturn = QOSRemoveSocketFromFlow(h->QoSHandle, (SOCKET)NULL, h->QoSFlowId, 0);
	}

	if (h->QoSHandle != NULL)
	{
		bReturn = QOSCloseHandle(h->QoSHandle);
	}

	/** This function calls the Winsock closesocket() API to actually close the
	    socket. */
	if (h->socket)
	{
		/** For TLS client, save SSL resources here. These would be released after recieve thread is exit */
		if (h->protocolType == ProtocolType_TLS_Client)
		{
#ifdef ENABLE_TLS_OPENSSL
			ssl = h->ssl;
			ctx = h->ctx;

			/** SSL handshake shutdown */
			if (ssl != NULL)
			{
				sslReturn = SSL_shutdown(ssl);
				/** Ignore the return value. */
				SOCKETLOGD(m->logHandle, KLogTypeGeneral, "%s:%u, SSL shutdown, sslReturn %d", __FUNCTION__, __LINE__, sslReturn);
			}
#else
			/** No SSL Library defined! it wouldnt appear since TLS socket couldnt be created without any SSL library */
			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,  No SSL Library is defined !",
				__FUNCTION__, __LINE__);
#endif /* ENABLE_TLS_OPENSSL */
		}

		socketReturn = shutdown(h->socket, SD_BOTH);
		/* Ignore the return value. */
		SOCKETLOGD(m->logHandle, KLogTypeGeneral, "%s:%u, shutdown, socketReturn %d", __FUNCTION__, __LINE__, socketReturn);

		socketReturn = closesocket(h->socket);
		/* Ignore the return value. */
		SOCKETLOGD(m->logHandle, KLogTypeGeneral, "%s:%u, close, socketReturn %d", __FUNCTION__, __LINE__, socketReturn);
	}

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);

	if (h->hRecvThread != NULL)
	{
		if (h->bEnableGlobalMutex)
		{
			pal_MutexGlobalUnlock(m->pal);
		}

		/** Wait for the Socket Receive thread to terminate. */
		dReturn = WaitForSingleObject(h->hRecvThread, INFINITE);
		/* Ignoring the return value. */
		/* @todo Note that waiting for the thread to exit is an infinite operation. */

		if (h->bEnableGlobalMutex)
		{
			pal_MutexGlobalLock(m->pal);
		}

		/* Cleanup the Socket Receive thread handle. */
		bReturn = CloseHandle(h->hRecvThread);
		/* Ignoring the return value. */

		h->hRecvThread = NULL;
	}

	if (h->hAcceptThread != NULL)
	{
		if (h->bEnableGlobalMutex)
		{
			pal_MutexGlobalUnlock(m->pal);
		}

		/** Wait for the Socket Accept thread to terminate. */
		dReturn = WaitForSingleObject(h->hAcceptThread, INFINITE);
		/* Ignoring the return value. */
		/* @todo Note that waiting for the thread to exit is an infinite operation. */

		if (h->bEnableGlobalMutex)
		{
			pal_MutexGlobalLock(m->pal);
		}

		/* Cleanup the Socket Accept thread handle. */
		bReturn = CloseHandle(h->hAcceptThread);
		/* Ignoring the return value. */

		h->hAcceptThread = NULL;
	}

	/** Enter the operational critical section (for the module). */
	EnterCriticalSection(&m->operationalCS);

	/** Remove the Socket from the module list. */
	ec_removeSocket(m, h);
	m->socketListSize--;

	/** Leave the operational critical section (for the module). */
	LeaveCriticalSection(&m->operationalCS);

	/** Delete the critical section. */
	DeleteCriticalSection(&h->socketCS);

	if (h->uNumOfFingerPrint > 0)
	{
		for (int j = 0; j < h->uNumOfFingerPrint; j++)
		{
			pal_MemoryFree((void **)&h->ppFingerPrint[j]);
		}
		pal_MemoryFree((void **)&h->ppFingerPrint);
	}

#ifdef ENABLE_TLS_OPENSSL
	/** Free SSL resources*/
	if (ssl != NULL)
	{
		SSL_free(ssl);
		h->ssl = NULL;
	}

	if (ctx != NULL)
	{
		SSL_CTX_free(ctx);
		h->ctx = NULL;
	}

#endif /* ENABLE_TLS_OPENSSL */

	if (h->pRecvBuffer != NULL)
	{
		/* Free the memory allocated for the receive buffer. */
		free(h->pRecvBuffer);
	}

	/** Free the memory allocated for the Socket instance. */
	free(h);

	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return KPALErrorNone;
}

/**
 * Implementation of the pal_SocketInit() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_socketInit
(
	PALINSTANCE pal,
	LOGHANDLE logHandle,
	SOCKETINSTANCE *instance
)
{
	SocketInstanceStruct *m = NULL;
	int socketReturn = 0;
	BOOL bReturn;
	unsigned int error = KPALErrorNone;

	if (instance == NULL)
	{
		return KPALInvalidParameters;
	}

	*instance = NULL;

	m = (SocketInstanceStruct *)malloc(sizeof(SocketInstanceStruct));
	if (m == NULL)
	{
		/* Memory for the Socket Module is not available. */
		return KPALMemoryError;
	}

	memset(m, 0, sizeof(SocketInstanceStruct));

	/* Initialize structure attributes. */
	m->initState = ECRIO_PAL_MODULE_STATE_UNINITIALIZED;

	m->pal = pal;
	m->logHandle = logHandle;

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	m->head = NULL;
	m->tail = NULL;
	m->socketListSize = 0;
	m->hThread = NULL;

	/** Initialize Winsock for the process, requesting version 2.2 if
	    available. WSAStartup() can be called multiple times per process,
	    but for each successful call made, a call to WSACleanup() is
	    required. */
	socketReturn = WSAStartup(MAKEWORD(2, 2), &m->wsaData);
	if (socketReturn != 0)
	{
		error = KPALInternalError;
		goto Error;
	}

#ifdef ENABLE_TLS_OPENSSL
	/** TLS/SSL startup */
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
#endif /* ENABLE_TLS_OPENSSL */

	/** Create an event that will be used to wait for the thread to complete. */
	m->hThreadReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m->hThreadReady == NULL)
	{
		error = KPALInternalError;
		goto Error;
	}

	/** Create the Module thread. */
	m->hThread = (HANDLE)_beginthreadex(NULL, 0, SocketModuleThread, (void *)m, FALSE, (unsigned int *)&(m->threadId));
	if (m->hThread == NULL)
	{
		error = KPALInternalError;
		goto Error;
	}

	/** Initialize the initialization critical section. */
	InitializeCriticalSection(&m->initCS);
	/* Critical sections are guaranteed by the OS to be initialized. */

	/** Initialize the operational critical section. */
	InitializeCriticalSection(&m->operationalCS);
	/* Critical sections are guaranteed by the OS to be initialized. */

	/* Wait for the thread to initialize before continuing. */
	WaitForSingleObject(m->hThreadReady, INFINITE);
	/* @todo Note that waiting for the thread to initialize is an infinite operation. */

	*instance = (SOCKETINSTANCE)m;

	/* Set the state as initialized. */
	m->initState = ECRIO_PAL_MODULE_STATE_INITIALIZED;

	SOCKETLOGI(logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;

Error:

	if (m != NULL)
	{
		if (m->hThreadReady != NULL)
		{
			/* Cleanup the Socket Module event handle. */
			bReturn = CloseHandle(m->hThreadReady);
			/* Ignoring the return value. */
		}

		free(m);
	}

	SOCKETLOGI(logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return error;
}

/**
 * Implementation of the pal_SocketDeinit() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
void pal_socketDeinit
(
	SOCKETINSTANCE instance
)
{
	SocketInstanceStruct *m = (SocketInstanceStruct *)instance;
	unsigned int returnValue = KPALErrorNone;
	SocketStruct *currentSocket = NULL;
	SocketStruct *nextSocket = NULL;
	BOOL bReturn;
	DWORD dReturn;

	if (m == NULL)
	{
		return;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   Deinit() would/should be called. This is a corner case if
		   the Calling Component erroneously called Deinit() multiple
		   times. */
		LeaveCriticalSection(&m->initCS);
		return;
	}

	/* Set the state to deinitializing to prevent any other interface
	   function from executing. */
	m->initState = ECRIO_PAL_MODULE_STATE_DEINITIALIZING;
	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&m->operationalCS);

	/** Loop for all sockets in the list so any that are still present are
	    properly cleaned up. This is useful in case pal_SocketClose() was
	    not called for all sockets. */
	currentSocket = m->head;

	while (currentSocket != NULL)
	{
		nextSocket = currentSocket->next;

		/* Leave the operational critical section. */
		LeaveCriticalSection(&m->operationalCS);

		/* Attempt to close the socket by calling the internal function to
		    close the socket and clean up resources. */
		returnValue = ec_socketClose(m, currentSocket);
		/* Ignoring the return value. */

		/* Enter the operational critical section. */
		EnterCriticalSection(&m->operationalCS);

		currentSocket = nextSocket;
	}

	if (m->socketListSize > 0)
	{
		/* There is some list corruption for some reason. */
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u, Module socket list is corrupt, remaining list size %d",
			__FUNCTION__, __LINE__, m->socketListSize);
	}

	/** Leave the operational critical section. */
	LeaveCriticalSection(&m->operationalCS);

	if (m->hThread != NULL)
	{
		/* Signal the Module thread to exit. */
		bReturn = PostThreadMessage(m->threadId, MODULE_THREAD_EXIT, 0, 0);
		/* Ignoring the return value. */

		/** Wait for the Module thread to terminate. */
		dReturn = WaitForSingleObject(m->hThread, INFINITE);
		/* Ignoring the return value. */
		/* @todo Note that waiting for the thread to exit is an infinite operation. */

		/* Cleanup the Module thread handle. */
		bReturn = CloseHandle(m->hThread);
		/* Ignoring the return value. */

		m->hThread = NULL;
	}

	if (m->hThreadReady != NULL)
	{
		/* Cleanup the Socket Module event handle. */
		bReturn = CloseHandle(m->hThreadReady);
		/* Ignoring the return value. */

		m->hThreadReady = NULL;
	}

	/** Delete the critical sections. */
	DeleteCriticalSection(&m->operationalCS);
	DeleteCriticalSection(&m->initCS);

	/** Cleanup Windows Sockets. */
	WSACleanup();

	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	/** Free the memory allocated for the Socket Module instance. */
	free(m);
}

/**
 * Implementation of the pal_SocketCreate() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketCreate
(
	PALINSTANCE pal,
	SocketCreateConfigStruct *pConfig,
	SocketCallbackFnStruct *pFnStruct,
	SOCKETHANDLE *handle
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
	unsigned int error = KPALErrorNone;

	int addressfamily = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;

	SocketTLSSettingsStruct *pTLSSetting = NULL;

	QOS_VERSION Version;
	BOOL QoSResult = FALSE;

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

	m = (SocketInstanceStruct *)((EcrioPALStruct *)pal)->socket;

	/* The pointer to the Module Handle must not be NULL. */
	if (m == NULL)
	{
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

#ifndef ENABLE_TLS_OPENSSL
	if (pConfig->protocolType == ProtocolType_TLS_Client)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u, No TLS library ",
			__FUNCTION__, __LINE__);

		error = KPALNotSupported;
		goto Error;
	}
#endif /* ENABLE_TLS_OPENSSL */

	if (pConfig->protocolType == ProtocolType_TLS_Client)
	{

		if (pConfig->tlsSetting == NULL)
		{
			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u, Wrong input , for TLS client tlsSetting MUST not NULL !",
				__FUNCTION__, __LINE__);

			error = KPALInternalError;
			goto Error;
		}
		
		pTLSSetting = pConfig->tlsSetting;
	}

	/** Allocate memory for this Socket. */
	h = (SocketStruct *)calloc(sizeof(SocketStruct), 1);
	if (h == NULL)
	{
		/* Memory for the Socket is not available. */
		error = KPALMemoryError;
		goto Error;
	}

	/* Initialize the state attributes. */
	h->bCreated = false;
	h->bCallbacksSet = false;
	h->bLocalHostSet = false;
	h->bRemoteHostSet = false;
	h->bRemotePortZero = false;
	h->bOpen = false;
	h->bClosed = false;

	h->hRecvThread = NULL;
	h->hAcceptThread = NULL;

	/* Set the socket receive buffer and allocate it. */
	h->bufferSize = pConfig->bufferSize;
	h->pRecvBuffer = NULL;

	/* Saving the information of socket type */
	h->protocolType = pConfig->protocolType;
	h->protocolVersion = pConfig->protocolVersion;
	h->socketType = pConfig->socketType;
	h->socketMediaType = pConfig->socketMediaType;

	h->bEnableGlobalMutex = pConfig->bEnableGlobalMutex;

	/* We don't allocate any receive buffer for TCP Servers. They only accept
	   incoming socket connections and never receive data, at least none that
	   callers need to be informed about. */
	if (h->protocolType != ProtocolType_TCP_Server)
	{
		h->pRecvBuffer = (char *)calloc(h->bufferSize, 1);
		if (h->pRecvBuffer == NULL)
		{
			/* Memory for the receive buffer is not available. */
			error = KPALMemoryError;
			goto Error;
		}
	}
	else
	{
		/* If the type is for a TCP Server, an Accept callback function is
		   required. Otherwise there is no point to waste resources for it. */
		if (pFnStruct->pAcceptCallbackFn == NULL)
		{
			/* Any previously set callbacks are not valid anymore. */
			h->bCallbacksSet = false;

			error = KPALNotAllowed;
			goto Error;
		}
	}

	/** Copy the callback data to the internal structure. We don't care if
	    this function has already been called before the Socket is opened. */
	h->callback.pReceiveCallbackFn = pFnStruct->pReceiveCallbackFn;
	h->callback.pErrorCallbackFn = pFnStruct->pErrorCallbackFn;
	h->callback.pAcceptCallbackFn = pFnStruct->pAcceptCallbackFn;
	h->callback.pCallbackFnData = pFnStruct->pCallbackFnData;

	h->bCallbacksSet = true;

	switch (h->protocolVersion)
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

	switch (h->protocolType)
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

	h->uNumOfFingerPrint = 0;
	h->ppFingerPrint = NULL;

	/** Create a socket by calling the socket() Winsock function. */
	h->socket = socket(addressfamily, type, protocol);
	if (h->socket == INVALID_SOCKET)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u, socket() failed, socketReturn %d socketError %d",
			__FUNCTION__, __LINE__, h->socket, WSAGetLastError());
		error = KPALInternalError;
		goto Error;
	}

	// @todo If the socet is for audio/video/text.

	// Initialize the QoS version parameter.
	Version.MajorVersion = 1;
	Version.MinorVersion = 0;

	h->QoSHandle = NULL;

	QoSResult = QOSCreateHandle(&Version, &h->QoSHandle);
	// @todo Check result.

	/** Initialize the socket operational critical section. */
	InitializeCriticalSection(&h->socketCS);
	/* Critical sections are guaranteed by the OS to be initialized. */

	if (pTLSSetting != NULL && (h->protocolType == ProtocolType_TLS_Client))
	{
		/** Enter the operational critical section for the socket. */
		EnterCriticalSection(&h->socketCS);
		/** Initialize TLS context */
		error = ec_tlsInitialize(m, h, pTLSSetting);
		/** Leave the operational critical section for the socket. */
		LeaveCriticalSection(&h->socketCS);

		if (error != KPALErrorNone)
		{
			/** Delete the socket operational critical sections. */			
			DeleteCriticalSection(&h->socketCS);

			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u, TLS initialize failed",
				__FUNCTION__, __LINE__);
			goto Error;
		}
	}

	/** Enter the operational critical section for the module. */
	EnterCriticalSection(&m->operationalCS);

	/** The new Socket is now initialized. Add it to the module list. */
	h->next = NULL;
	ec_addSocket(m, h);
	m->socketListSize++;

	/* Remember the module that the Socket is part of. */
	h->module = (SOCKETINSTANCE)m;

	/** Leave the operational critical section for the module. */
	LeaveCriticalSection(&m->operationalCS);

	/** Initialize the socket operational critical section. */
	InitializeCriticalSection(&h->socketCS);
	/* Critical sections are guaranteed by the OS to be initialized. */

	/* The socket is now created. */
	h->bCreated = true;

	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	*handle = (SOCKETHANDLE)h;
	return error;

Error:

	/* Currently, can only get here if the allocation failed or the socket
	   creation failed. Hence, there is no need to close the socket or
	   reverse other things that have been done after the socket was
	   created. */
	if (h != NULL)
	{
		if (h->pRecvBuffer != NULL)
		{
			/* Free the memory allocated for the receive buffer. */
			free(h->pRecvBuffer);
		}

		/* Free the memory allocated for the socket. */
		free(h);

		h = NULL;
	}

	*handle = h;

	return error;
}

/**
 * Implementation of the pal_SocketSetLocalHost() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketSetLocalHost
(
	SOCKETHANDLE handle,
	const u_char *pLocalIP,
	u_int16 localPort
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
	unsigned int returnValue = KPALErrorNone;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (SocketStruct *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	/** If the Socket is open, we can't change the local host. The Calling
	    Component is expected to close and re-open the Socket. */
	if (h->bOpen)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u, (0x%08x) Socket is already open",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALAlreadyInUse;
		goto Error;
	}

	/** Set the socket address information for this host. We don't care if the
	    host has already been set or not (before the Socket has been open).
	    The IP String can be point to a valid IP address. Or, if it is NULL
	    or set to an "INADDR_ANY" or in6addr_any, the bind mechanism should
	    choose the IP address. */
	returnValue = ec_setHostInfo(h, (const char *)pLocalIP, localPort, true, &h->local, &h->localSize);
	if (returnValue != KPALErrorNone)
	{
		/* The IP string was probably invalid. */
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u, (0x%08x) ec_setHostInfo() failed",
			__FUNCTION__, __LINE__, h);

		/* Even though the ec_setHostInfo() function won't modify the local host
		   information if there is an error, we will treat the local host as
		   not valid anymore because we need to keep the Calling Component in
		   sync. */
		h->bLocalHostSet = false;

		goto Error;
	}

	/* We now have a valid local host. */
	h->bLocalHostSet = true;

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return KPALErrorNone;

Error:

	/* Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return returnValue;
}

/**
 * Implementation of the pal_SocketSetRemoteHost() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketSetRemoteHost
(
	SOCKETHANDLE handle,
	const u_char *pRemoteIP,
	u_int16 remotePort
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;

	BOOL QoSResult = FALSE;

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
	h = (SocketStruct *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	/** If the Socket is open, we can't change the remote host for TCP. The
	    Calling Component is expected to close and re-create the Socket. */
	if ((h->protocolType != ProtocolType_UDP) &&
		(h->bOpen))
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u, (0x%08x) Socket is already open",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALAlreadyInUse;
		goto Error;
	}

	if ((h->protocolType == ProtocolType_UDP) ||
		(h->protocolType == ProtocolType_TCP_Client) ||
		(h->protocolType == ProtocolType_TLS_Client))		
	{
		/** Set the socket address information for this host. We don't care if the
		    host has already been set or not (before the Socket has been open). */
		returnValue = ec_setHostInfo(h, (const char *)pRemoteIP, remotePort, false, &h->remote, &h->remoteSize);
		if (returnValue != KPALErrorNone)
		{
			/* The IP string was probably invalid. */
			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) ec_setHostInfo() failed",
				__FUNCTION__, __LINE__, h);

			/* Even though the ec_setHostInfo() function won't modify the remote host
			   information if there is an error, we will treat the remote host as
			   not valid anymore because we need to keep the Calling Component in
			   sync. */
			h->bRemoteHostSet = false;
			h->bRemotePortZero = false;

			goto Error;
		}

		if (remotePort == 0)
		{
			if ((h->protocolType == ProtocolType_TCP_Client) ||
				(h->protocolType == ProtocolType_TLS_Client))
			{
				/* Remote port can't be 0 for TCP clients. There is no recovery
				   to a prior state, so ensure that no other functions can
				   succeed. However, this function can be called again. */
				h->bRemoteHostSet = false;
				h->bRemotePortZero = false;

				returnValue = KPALNotSupported;
				goto Error;
			}
			else
			{
				/** We will track if the port is set to zero, which allows it to
				    be a wildcard, but it also means we don't have a singular
				    remote host to send data to, but we can still connect, even
				    for UDP. */
				h->bRemotePortZero = true;
			}
		}

		/* We now have a valid remote host. */
		h->bRemoteHostSet = true;

		/** For UDP, we will re-establish the connection if the connection is
		    open. */
		if ((h->protocolType == ProtocolType_UDP) &&
			(h->bOpen))
		{
			/** Associate the configured remote host information with the
			    socket by calling the connect() API. This is valid for UDP
			    protocols because it will establish the default destination
			    address for subsequent calls to send/recv. */
			socketReturn = connect(h->socket, (struct sockaddr *)&h->remote, h->remoteSize);
			if (socketReturn == SOCKET_ERROR)
			{
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x) connect() failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

				h->bRemoteHostSet = false;
				h->bRemotePortZero = false;

				returnValue = KPALInternalError;
				goto Error;
			}

			/* Setup QoS flow (the DSCP bits) if the socket is used for various media
			   streams. We will use the non-adaptive algorithm and just let all the
			   packets be marked with the priority, and hope that the router elements
			   support WMM and/or other diffserv abilities. */
			if ((h->socketMediaType == SocketMediaType_Audio) &&
				(h->socketType == SocketType_RTP))
			{
				// @todo What if we already set it? We need to clean it up somehow.
				h->QoSFlowId = 0;

				QoSResult = QOSAddSocketToFlow(
					h->QoSHandle,
					h->socket,
					NULL,
					QOSTrafficTypeVoice,
					QOS_NON_ADAPTIVE_FLOW,
					&h->QoSFlowId);
				if (QoSResult != TRUE)
				{
					SOCKETLOGW(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) QOSAddSocketToFlow() failed, socketError %d",
						__FUNCTION__, __LINE__, h, WSAGetLastError());
				}
			}
			else if ((h->socketMediaType == SocketMediaType_Video) &&
				(h->socketType == SocketType_RTP))
			{
				// @todo What if we already set it? We need to clean it up somehow.
				h->QoSFlowId = 0;

				QoSResult = QOSAddSocketToFlow(
					h->QoSHandle,
					h->socket,
					NULL,
					QOSTrafficTypeAudioVideo,
					QOS_NON_ADAPTIVE_FLOW,
					&h->QoSFlowId);
				if (QoSResult != TRUE)
				{
					SOCKETLOGW(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) QOSAddSocketToFlow() failed, socketError %d",
						__FUNCTION__, __LINE__, h, WSAGetLastError());
				}
			}
			else if (h->socketType == SocketType_RTCP)
			{
				// @todo What if we already set it? We need to clean it up somehow.
				h->QoSFlowId = 0;

				// @todo Not sure if RTCP should be give control priority... Going with
				// it for now.
				QoSResult = QOSAddSocketToFlow(
					h->QoSHandle,
					h->socket,
					NULL,
					QOSTrafficTypeControl,
					QOS_NON_ADAPTIVE_FLOW,
					&h->QoSFlowId);
				if (QoSResult != TRUE)
				{
					SOCKETLOGW(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) QOSAddSocketToFlow() failed, socketError %d",
						__FUNCTION__, __LINE__, h, WSAGetLastError());
				}
			}
		}
	}
	else if (h->protocolType == ProtocolType_TCP_Server)
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
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u, (0x%08x) Invalid protocol",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALInsufficientParameters;
		goto Error;
	}

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return KPALErrorNone;

Error:

	/* Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return returnValue;
}

/**
 * Implementation of the pal_SocketGetLocalHost() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketGetLocalHost
(
	SOCKETHANDLE handle,
	u_char *pLocalIP,
	u_int16 *pLocalPort
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
	SOCKADDR_STORAGE Addr = {0};
	int AddrLen = 0;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;

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
	h = (SocketStruct *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	if (h->bLocalHostSet)
	{
		if (h->bOpen)
		{
			/** The socket is currently open. Use getsockname() to find out
			    the actual socket details in case the operating system had to
			    choose the IP and/or port. */
			AddrLen = sizeof(Addr);

			socketReturn = getsockname(h->socket, (struct sockaddr *)&Addr, &AddrLen);
			if (socketReturn == SOCKET_ERROR)
			{
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u, (0x%08x) getsockname() failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

				returnValue = KPALInternalError;
				goto Error;
			}
		}
		else
		{
			/** The socket has not been opened yet, so we will simply
			    reflect the IP and port that were set by the Calling
			    Component. */
			AddrLen = h->localSize;

			/* Copy the address to the destination structure. */
			memcpy(&Addr, &h->local, AddrLen);
		}

		/** Get the IP and port from the sockaddr_storage structure, so it will
		    be read as either IPv4 or IPv6 by using getnameinfo(). */
		socketReturn = getnameinfo((struct sockaddr *)&Addr, AddrLen, (char *)pLocalIP, SOCKET_IP_SIZE, NULL, 0, NI_NUMERICHOST);
		if (socketReturn != 0)
		{
			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) getnameinfo() failed, socketReturn %d socketError %d",
				__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

			returnValue = KPALInternalError;
			goto Error;
		}

		*pLocalPort = ntohs(((struct sockaddr_in *)(&Addr))->sin_port);
	}
	else
	{
		/* The local host hasn't been set yet. */
		returnValue = KPALEntityInvalid;
		goto Error;
	}

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return KPALErrorNone;

Error:

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return returnValue;
}

/**
 * Implementation of the pal_SocketGetRemoteHost() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketGetRemoteHost
(
	SOCKETHANDLE handle,
	u_char *pRemoteIp,
	u_int16 *pRemotePort
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
	SOCKADDR_STORAGE Addr = {0};
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
	h = (SocketStruct *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	if ((h->protocolType == ProtocolType_UDP) ||
		(h->protocolType == ProtocolType_TCP_Client) ||
		(h->protocolType == ProtocolType_TLS_Client))
	{
		if (h->bRemoteHostSet)
		{
			if (h->bOpen)
			{
				/** The socket is currently open. Use getpeername to find out
				    the actual socket details in case the operating system had
				    to choose the IP and/or port. */
				AddrLen = sizeof(Addr);

				socketReturn = getpeername(h->socket, (struct sockaddr *)&Addr, &AddrLen);
				if (socketReturn == SOCKET_ERROR)
				{
					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) getpeername() failed, socketReturn %d socketError %d",
						__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

					returnValue = KPALInternalError;
					goto Error;
				}
			}
			else
			{
				/** The socket has not been opened yet, so we will simply
				    reflect the IP and port that were set by the Calling
				    Component. */
				AddrLen = h->remoteSize;

				/* Copy the address to the destination structure. */
				memcpy(&Addr, &h->remote, AddrLen);
			}

			/** Get the IP and port from the sockaddr_storage structure, so it will
			    be read as either IPv4 or IPv6 by using getnameinfo(). */
			socketReturn = getnameinfo((struct sockaddr *)&Addr, AddrLen, (char *)pRemoteIp, SOCKET_IP_SIZE, NULL, 0, NI_NUMERICHOST);
			if (socketReturn != 0)
			{
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x) getnameinfo() failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

				returnValue = KPALInternalError;
				goto Error;
			}

			*pRemotePort = ntohs(((struct sockaddr_in *)(&Addr))->sin_port);
		}
		else
		{
			/* The remote host hasn't been set. */
			returnValue = KPALEntityInvalid;
			goto Error;
		}
	}
	else if (h->protocolType == ProtocolType_TCP_Server)
	{
		/* Getting a remote host for a TCP Server is not allowed. A TCP Server
		   does not need a remote host. */
		returnValue = KPALNotSupported;
		goto Error;
	}
	else
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) Invalid protocol",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALInsufficientParameters;
		goto Error;
	}

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return KPALErrorNone;

Error:

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return returnValue;
}

/**
 * Implementation of the pal_SocketOpen() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketOpen
(
	SOCKETHANDLE handle
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;

	BOOL bOptVal = TRUE;
	int bOptLen;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (SocketStruct *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	/** The socket can't already be open. */
	if (h->bOpen)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) Socket is already open",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALAlreadyInUse;
		goto Error;
	}

	/** As long as the socket is created, the callbacks have been set, and the
	    local host is set, we can open the socket. The fact that it is created
	    is by definition. If any of these conditions aren't met, return an
	    error. */
	if ((!h->bCreated) ||
		(!h->bCallbacksSet) ||
		(!h->bLocalHostSet))
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) Socket is not created, has no callbacks set, or has no local host set",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	if (((h->protocolType == ProtocolType_TCP_Client) ||
		(h->protocolType == ProtocolType_TLS_Client) ) &&
		(!h->bRemoteHostSet))
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) Remote host is not set for TCP Client",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	/** check if TLS is properly initialized for this socket*/
	if ((h->protocolType == ProtocolType_TLS_Client ) 
		&& h->bTLSInit == false)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) TLS is not initialized for this socket",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	bOptLen = sizeof(bOptVal);
	socketReturn = setsockopt(h->socket, SOL_SOCKET, SO_REUSEADDR, (char *)&bOptVal, bOptLen);
	if (socketReturn == SOCKET_ERROR)
	{
		/* Although this function returned a failure, it isn't fatal, socket
		   operations might still be possible. */
		SOCKETLOGD(m->logHandle, KLogTypeGeneral,
			"%s:%u, setsockopt() failed, socketReturn %d socketError %d",
			__FUNCTION__, __LINE__, socketReturn, WSAGetLastError());
	}
	else
	{
		/* @todo We should really check to be sure we set it or else let the caller know, not a priority though. */
	}

	/** Associate the configured local host information with the socket by
	    calling the bind() API. */
	socketReturn = bind(h->socket, (struct sockaddr *)&h->local, h->localSize);
	if (socketReturn == SOCKET_ERROR)
	{
		SOCKETLOGD(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) bind() failed, socketReturn %d socketError %d",
			__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

		returnValue = KPALInternalError;
		goto Error;
	}

	if (h->protocolType == ProtocolType_TCP_Server)
	{
		/** Set the socket to listen for incoming connections. */
		socketReturn = listen(h->socket, SOMAXCONN);
		if (socketReturn == SOCKET_ERROR)
		{
			SOCKETLOGD(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) listen() failed, socketReturn %d socketError %d",
				__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

			returnValue = KPALInternalError;
			goto Error;
		}

		/** Create the Socket TCP Accept thread. */
		h->hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, SocketAcceptThread, (void *)h, FALSE, (unsigned int *)&(h->acceptThreadId));
		if (h->hAcceptThread == NULL)
		{
			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) CreateThread(accept thread) failed",
				__FUNCTION__, __LINE__, h);

			returnValue = KPALInternalError;
			goto Error;
		}
	}
	else
	{
		if (h->bRemoteHostSet == true)
		{
			/** Associate the configured remote host information with the
			    socket by calling the connect() API. This is also valid for
			    UDP protocols because it will establish the default destination
			    address for subsequent calls to send/recv. */
			socketReturn = connect(h->socket, (struct sockaddr *)&h->remote, h->remoteSize);
			if (socketReturn == SOCKET_ERROR)
			{
				SOCKETLOGD(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x) connect() failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

				returnValue = KPALInternalError;
				goto Error;
			}

			/** For TLS socket, do connection/handshake with TLS server */
			if (h->protocolType == ProtocolType_TLS_Client) 
			{
				/** Connect using TLS */
				returnValue = ec_tlsConnect(h);

				if (returnValue != KPALErrorNone )
				{
					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) TLS connection failed",
						__FUNCTION__, __LINE__, h);

					goto Error;
				}
				else
				{
					/** Check peer certificate */
					returnValue = ec_tlsServerCertificate(h);

					if (returnValue != KPALErrorNone)
					{
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
							"%s:%u,(0x%08x) TLS peer certificate verification failed",
							__FUNCTION__, __LINE__, h);
						goto Error;
					}
				}
			}
		}

		if (h->callback.pReceiveCallbackFn != NULL)
		{
			/** Create the Socket Receive thread only if there is a receive
			    callback specified. */
			h->hRecvThread = (HANDLE)_beginthreadex(NULL, 0, SocketRecvThread, (void *)h, FALSE, (unsigned int *)&(h->recvThreadId));
			if (h->hRecvThread == NULL)
			{
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x) CreateThread(receive thread) failed",
					__FUNCTION__, __LINE__, h);

				returnValue = KPALInternalError;
				goto Error;
			}
		}
		else
		{
			/* Ensure that h->hRecvThread is NULL if not used. */
			h->hRecvThread = NULL;
		}
	}

	/* The socket is now open. */
	h->bOpen = true;

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return KPALErrorNone;

Error:

	/* Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);

	/** The Calling Component must call pal_SocketClose() to properly clean up
	    all threads and resources in case of error. */
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return returnValue;
}

/**
 * Implementation of the pal_SocketOpenEx() function. 
 * bind() is invoked only for TCP Server sockte. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketOpenEx
(
	SOCKETHANDLE handle
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
	unsigned int returnValue = KPALErrorNone;
	int socketReturn = 0;

	BOOL bOptVal = TRUE;
	int bOptLen;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return KPALInvalidHandle;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (SocketStruct *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	/** The socket can't already be open. */
	if (h->bOpen)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) Socket is already open",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALAlreadyInUse;
		goto Error;
	}

	/** As long as the socket is created, the callbacks have been set, and the
	    local host is set, we can open the socket. The fact that it is created
	    is by definition. If any of these conditions aren't met, return an
	    error. */
	if ((!h->bCreated) ||
		(!h->bCallbacksSet) ||
		(!h->bLocalHostSet))
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) Socket is not created, has no callbacks set, or has no local host set",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	if (((h->protocolType == ProtocolType_TCP_Client) ||
		(h->protocolType == ProtocolType_TLS_Client) ) &&
		(!h->bRemoteHostSet))
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) Remote host is not set for TCP Client",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	/** check if TLS is properly initialized for this socket*/
	if ((h->protocolType == ProtocolType_TLS_Client ) 
		&& h->bTLSInit == false)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) TLS is not initialized for this socket",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	bOptLen = sizeof(bOptVal);
	socketReturn = setsockopt(h->socket, SOL_SOCKET, SO_REUSEADDR, (char *)&bOptVal, bOptLen);
	if (socketReturn == SOCKET_ERROR)
	{
		/* Although this function returned a failure, it isn't fatal, socket
		   operations might still be possible. */
		SOCKETLOGD(m->logHandle, KLogTypeGeneral,
			"%s:%u, setsockopt() failed, socketReturn %d socketError %d",
			__FUNCTION__, __LINE__, socketReturn, WSAGetLastError());
	}
	else
	{
		/* @todo We should really check to be sure we set it or else let the caller know, not a priority though. */
	}

	/** Associate the configured local host information with the socket by
	    calling the bind() API. */
	if (h->protocolType == ProtocolType_TCP_Server){
		socketReturn = bind(h->socket, (struct sockaddr *)&h->local, h->localSize);
		if (socketReturn == SOCKET_ERROR)
		{
			SOCKETLOGD(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) bind() failed, socketReturn %d socketError %d",
				__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

			returnValue = KPALInternalError;
			goto Error;
		}
	}

	if (h->protocolType == ProtocolType_TCP_Server)
	{
		/** Set the socket to listen for incoming connections. */
		socketReturn = listen(h->socket, SOMAXCONN);
		if (socketReturn == SOCKET_ERROR)
		{
			SOCKETLOGD(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) listen() failed, socketReturn %d socketError %d",
				__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

			returnValue = KPALInternalError;
			goto Error;
		}

		/** Create the Socket TCP Accept thread. */
		h->hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, SocketAcceptThread, (void *)h, FALSE, (unsigned int *)&(h->acceptThreadId));
		if (h->hAcceptThread == NULL)
		{
			SOCKETLOGE(m->logHandle, KLogTypeGeneral,
				"%s:%u,(0x%08x) CreateThread(accept thread) failed",
				__FUNCTION__, __LINE__, h);

			returnValue = KPALInternalError;
			goto Error;
		}
	}
	else
	{
		if (h->bRemoteHostSet == true)
		{
			/** Associate the configured remote host information with the
			    socket by calling the connect() API. This is also valid for
			    UDP protocols because it will establish the default destination
			    address for subsequent calls to send/recv. */
			socketReturn = connect(h->socket, (struct sockaddr *)&h->remote, h->remoteSize);
			if (socketReturn == SOCKET_ERROR)
			{
				SOCKETLOGD(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x) connect() failed, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, h, socketReturn, WSAGetLastError());

				returnValue = KPALInternalError;
				goto Error;
			}

			/** For TLS socket, do connection/handshake with TLS server */
			if (h->protocolType == ProtocolType_TLS_Client) 
			{
				/** Connect using TLS */
				returnValue = ec_tlsConnect(h);

				if (returnValue != KPALErrorNone )
				{
					SOCKETLOGE(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) TLS connection failed",
						__FUNCTION__, __LINE__, h);

					goto Error;
				}
				else
				{
					/** Check peer certificate */
					returnValue = ec_tlsServerCertificate(h);

					if (returnValue != KPALErrorNone)
					{
						SOCKETLOGE(m->logHandle, KLogTypeGeneral,
							"%s:%u,(0x%08x) TLS peer certificate verification failed",
							__FUNCTION__, __LINE__, h);
						goto Error;
					}
				}
			}
		}

		if (h->callback.pReceiveCallbackFn != NULL)
		{
			/** Create the Socket Receive thread only if there is a receive
			    callback specified. */
			h->hRecvThread = (HANDLE)_beginthreadex(NULL, 0, SocketRecvThread, (void *)h, FALSE, (unsigned int *)&(h->recvThreadId));
			if (h->hRecvThread == NULL)
			{
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u,(0x%08x) CreateThread(receive thread) failed",
					__FUNCTION__, __LINE__, h);

				returnValue = KPALInternalError;
				goto Error;
			}
		}
		else
		{
			/* Ensure that h->hRecvThread is NULL if not used. */
			h->hRecvThread = NULL;
		}
	}

	/* The socket is now open. */
	h->bOpen = true;

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return KPALErrorNone;

Error:

	/* Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);

	/** The Calling Component must call pal_SocketClose() to properly clean up
	    all threads and resources in case of error. */
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return returnValue;
}

/**
 * Implementation of the pal_SocketSendData() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketSendData
(
	SOCKETHANDLE handle,
	const u_char *pDataBuffer,
	u_int32 dataLength
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
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
	h = (SocketStruct *)handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->socketCS);

	/* @todo Need to figure out when we can leave the critical section! We shouldn't
	keep it going for the whole send process, correct? Shall we make an assumption
	that the caller can't kill the socket handle while we might block here??? */

	/** The socket must be open. */
	if (!h->bOpen)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) the socket must be open",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALEntityInvalid;
		goto Error;
	}

	/** One can't send data from a TCP Server. Only the TCP Client Socket of
	    an accepted TCP connection to a TCP Server can. */
	if (h->protocolType == ProtocolType_TCP_Server)
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) TCP Server can't send data",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotSupported;
		goto Error;
	}

	/** In order to send without remote information, the remote host must have
	    already been provided. But if it has a port set to 0 (zero), it is not
	    a valid remote host to send data to (the Calling Component must call
	    the pal_SocketSendDataTo() function instead.) */
	if ((!h->bRemoteHostSet) ||
		(h->bRemotePortZero))
	{
		SOCKETLOGE(m->logHandle, KLogTypeGeneral,
			"%s:%u,(0x%08x) no remote host set or remote port is 0",
			__FUNCTION__, __LINE__, h);

		returnValue = KPALNotAllowed;
		goto Error;
	}

	/** All bytes requested to be sent must be sent unless an error
	    is found. We will return success if all bytes are sent in
	    one or more calls, otherwise an error, even for UDP. */
	bytesSent = 0;
	bytesLeft = dataLength;

	if (h->protocolType == ProtocolType_TLS_Client)
	{
		/** Write buffer using TLS */
		returnValue = ec_tlsWrite(h, (const char *)&pDataBuffer[bytesSent], bytesLeft, &socketReturn);


		if (returnValue != KPALErrorNone)
		{
			if (returnValue == KPALDataSendErrorBenign)
			{
				/* These errors are benign and won't bring down the
				socket, so the Calling Component will be informed
				of the error, and it can decide to continue sending
				packets, as they may work, or close the Socket. */

				SOCKETLOGD(m->logHandle, KLogTypeGeneral,
					"%s:%u, benign send error, socketError %d",
					__FUNCTION__, __LINE__, WSAGetLastError());
				goto Error;
			}
			else
			{
				/* The Calling Component should close the socket and
				either give up or try again by creating a new one. */


				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u, fatal send error, socketError %d",
					__FUNCTION__, __LINE__, WSAGetLastError());
				goto Error;
			}
		}

	}
	else
	{
		if (dataLength == 0)
		{
			/* Send an empty packet. */
			socketReturn = send(h->socket, NULL, 0, 0);
		}
		else
		{
			while (bytesSent < dataLength)
			{
				/* Send to the remote host already set and connected to. */
				socketReturn = send(h->socket, (const char *)&pDataBuffer[bytesSent], bytesLeft, 0);
				if (socketReturn == SOCKET_ERROR)
				{
					/** Break out of the sending loop. */
					break;
				}

				bytesSent += socketReturn;
				bytesLeft -= socketReturn;

#ifdef ENABLE_LOG
				if (bytesSent < dataLength)
				{
					SOCKETLOGV(m->logHandle, KLogTypeGeneral,
						"%s:%u,(0x%08x) partial send!",
						__FUNCTION__, __LINE__, h);
				}
#endif

				/* @todo It might be prudent to have a counter and limit
				   the number of times we try to send data. If we reach
				   the limit, a benign error could be sent to the Calling
				   Component. */
			}
		}

		if (socketReturn == SOCKET_ERROR)
		{
			/* Conserve the error value. */
			socketError = WSAGetLastError();

			if ((socketError == WSAEWOULDBLOCK) ||
				(socketError == WSAEHOSTUNREACH) ||
				(socketError == WSAEFAULT) ||
				(socketError == WSAENOBUFS) ||
				(socketError == WSAEMSGSIZE) ||
				(socketError == WSAETIMEDOUT))
			{
				/* These errors are benign and won't bring down the
				   socket, so the Calling Component will be informed
				   of the error, and it can decide to continue sending
				   packets, as they may work, or close the Socket. */
				SOCKETLOGD(m->logHandle, KLogTypeGeneral,
					"%s:%u, benign send error, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, socketReturn, socketError);

				returnValue = KPALDataSendErrorBenign;
				goto Error;
			}
			else
			{
				/* The Calling Component should close the socket and
				   either give up or try again by creating a new one. */
				SOCKETLOGE(m->logHandle, KLogTypeGeneral,
					"%s:%u, fatal send error, socketReturn %d socketError %d",
					__FUNCTION__, __LINE__, socketReturn, socketError);

				returnValue = KPALDataSendErrorFatal;
				goto Error;
			}
		}
	}

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return KPALErrorNone;

Error:

	/* Leave the operational critical section. */
	LeaveCriticalSection(&h->socketCS);
	SOCKETLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return returnValue;
}

/**
 * Implementation of the pal_SocketClose() function. See the
 * EcrioPAL_Socket.h file for interface definitions.
 */
u_int32 pal_SocketClose
(
	SOCKETHANDLE *handle
)
{
	SocketInstanceStruct *m = NULL;
	SocketStruct *h = NULL;
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
	h = (SocketStruct *)*handle;

	/* Set the module handle pointer to the proper internal pointer. */
	m = (SocketInstanceStruct *)h->module;

	/* The Module Handle pointer must not be NULL. */
	if (m == NULL)
	{
		/* The Module Handle is not valid, therefore, the Socket is not
		   valid. */
		return KPALInvalidHandle;
	}

	SOCKETLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/* Enter the initialization critical section. */
	EnterCriticalSection(&m->initCS);
	if (m->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&m->initCS);
		return KPALNotInitialized;
	}

	LeaveCriticalSection(&m->initCS);

	/** Call the internal function ec_socketClose() to actually clean up all
	    the resources. */
	returnValue = ec_socketClose(m, h);

	/** Set the output parameter to NULL, to force NULL in the Calling
	    Component's context. */
	*handle = NULL;
	return returnValue;
}
