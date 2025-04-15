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
 * @file EcrioPAL_Socket.h
 * @brief This is the interface of the Ecrio PAL's Socket Module.
 *
 * The Socket Module of the Ecrio PAL supports the interfaces outlined in
 * this document. The Socket Module defines a number of functions meant
 * to closely map to standard socket functions, including those of the
 * standard Berkeley Socket and Microsoft Winsock APIs, in order to setup
 * and use UDP/TCP datagram sockets. However, the expected behavior is slightly
 * different and is described in the details for each API. The following list
 * gives the Socket Module APIs and the corresponding traditional socket
 * API it is meant to mimic:
 *
 * - pal_SocketInit() (for Winsock, WSAStartup())
 * - pal_SocketDeinit() (for Winsock, WSACleanup())
 * - pal_SocketCreate() (socket())
 * - pal_SocketSetOption() (getsockopt() if needed, setsockopt() if needed)
 * - pal_SocketSetCallbacks() (none)
 * - pal_SocketSetLocalHost() (might set a SOCKADDR_STORAGE structure)
 * - pal_SocketSetRemoteHost() (might set a SOCKADDR_STORAGE structure or call connect())
 * - pal_SocketGetLocalHost() (might call getsockname())
 * - pal_SocketGetRemoteHost() (might call getpeername())
 * - pal_SocketOpen(bind(), connect())
 * - pal_SocketSendData() (send())
 * - pal_SocketClose() (closesocket())
 *
 * Additionally, based on how the socket gets configured, either the socket
 * recv() or recvfrom() functions can be called. The socket "open" function,
 * pal_SocketOpen(), is really just the last formality to bind the local
 * socket so that the socket can be used. Note that while UDP datagrams are
 * connectionless in nature, some socket APIs allow the connect function to
 * be used, which would only affect the local socket, possibly making
 * subsequent calls to send packets more efficient.
 *
 * The Socket Module deals with hosts, which are a combination of a host's
 * IP address and port. In the interfaces, an IP address is always defined as a
 * character string that is expected to be in dot-decimal notation.
 *
 * In a normal scenario, the following sequence of function calls would be
 * expected from a Calling Component:
 *
 * - pal_SocketInit()
 * - pal_SocketCreate()
 * - pal_SocketSetCallbacks()
 * - pal_SocketSetLocalHost()
 * - pal_SocketSetRemoteHost()
 * - pal_SocketOpen()
 * - ...
 * - pal_SocketSendData|Sync() or pal_SocketSendDataTo|Sync()
 * - ...
 * - pal_SocketClose
 * - pal_SocketDeinit
 *
 * For each call to pal_SocketSendData() or pal_SocketSendDataTo(), the
 * SocketSendCallback() function would be invoked, if provided by the
 * Calling Component. In general, if the remote host is set via the call to
 * pal_SocketSetRemoteHost(), the pal_SocketSendData() function should
 * be called.
 *
 * After the pal_SocketOpen() function is called and returns successfully,
 * the SocketReceiveCallback() function can be invoked at any time there is
 * an incoming packet detected, usually via the socket recv() or recvfrom()
 * APIs. Again, recv() could be used if the remote host is known, or else
 * recvfrom() could be used if the remote host is not known.
 *
 * The following functions should not be called after the pal_SocketOpen()
 * is called (or they will return an error):
 *
 * - pal_SocketSetCallbacks()
 * - pal_SocketSetLocalHost()
 *
 * If callbacks or the local host information changes during the
 * lifetime of the Calling Component, then the socket should be closed
 * and created again.
 *
 * The pal_SocketSetRemoteHost() can be called after the pal_SocketOpen()
 * is called. Typically, this would invoke the socket API's connect() function
 * to re-establish a default connection with a new peer.
 *
 * The pal_SocketGetLocalHost() and pal_SocketGetRemoteHost() functions
 * can be called at any time. However they might behave differently before
 * and after the socket is opened. For instance, if the
 * pal_SocketSetLocalHost() function is specified with a wildcard
 * IP address or port (i.e. a NULL, empty, or 0.0.0.0 IP address or a
 * port value of 0), then the actual local host information would not
 * be known until it was bound.
 *
 * The Socket Module is instance based, meaning that multiple sockets are
 * supported. After calling the pal_SocketInit() function, multiple sockets
 * can be created by calling the pal_SocketCreate() function.
 *
 * <b>Multithreading</b>
 *
 * On platforms that support multithreading, all Socket Module API
 * functions are thread safe. The send, receive, and error callbacks will be
 * invoked from within dedicated threads that are synchronized. Additionally,
 * in situations that require it, the Socket Module APIs can be invoked
 * from different threads, accessing the same Socket Module instance,
 * without fail.
 *
 * In either the send or receive callbacks, the pal_SocketClose() function
 * must not be called, or else deadlock will ensue. The pal_SocketClose()
 * function can be called in response to other events, and from the error
 * callback. Typically, the error callback will indicate fatal errors
 * whilst errors reported via the send and receive callbacks are usually
 * benign or momentary.
 *
 * <b>Logging</b>
 *
 * Since the Socket Module supports multiple instances via multiple calls
 * to the pal_SocketInit() function, a handle of the type LOGHANDLE can
 * be specified so that the Socket Module can output logs to the same log
 * file as the Calling Component. This functionality is implementation
 * specific. Logs might not be supported at all or they could be written to a
 * separate physical log file.
 */

#ifndef __ECRIOPAL_SOCKET_H__
#define __ECRIOPAL_SOCKET_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Include the Ecrio PAL Logging Module definition as a direct dependency. */
#include "EcrioPAL_Log.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Module Handles
 * \n Handles are void pointers which will point to structures known
 * only internally to the Module. The Calling Component should only
 * reference the returned handles by these defined types. The handles
 * supported by this module include:
 */
/*@{*/

/**
 * A definition for the Socket instance, for use internally by the PAL.
 */
typedef void *SOCKETINSTANCE;

/**
 * The Socket handle refers to a single socket instance used for both
 * sending and receiving data and is created after calling
 * pal_SocketCreate(). A Socket Module can track multiple sockets, hence
 * the need for individual socket handles.
 */
typedef void *SOCKETHANDLE;

/*@}*/

/** @enum ProtocolVersionEnum
 * Enumeration holding the information of protocol version.
 */
typedef enum
{
	ProtocolVersion_IPv4			= 0,	/**< IPv4 Protocol Version. */
	ProtocolVersion_IPv6			= 1,	/**< IPv6 Protocol Version. */
	ProtocolVersion_IPv6_DualStack	= 2		/**< To support Dual stack Protocol Version. */
} ProtocolVersionEnum;

/** @enum ProtocolTypeEnum
 * Enumeration holding the information of protocol type.
 */
typedef enum
{
	ProtocolType_UDP		= 0,	/**< UDP Protocol Type. */
	ProtocolType_TCP_Client = 1,	/**< TCP with Client */
	ProtocolType_TCP_Server = 2,		/**< TCP with Server. */
	ProtocolType_TLS_Client = 3     /**< TLS TCP with Client */
} ProtocolTypeEnum;

/** @enum SocketOptionEnum
 * Enumeration holding various socket options that can be set.
 */
typedef enum
{
	SocketOption_Reuse_Address = 0,			/**< Sets the socket option SO_REUSEADDR to be set, a bool type. */
	SocketOption_Send_Buffer_Size,			/**< Sets the socket option SO_SNDBUF or similar, an int type. */
	SocketOption_Receive_Buffer_Size		/**< Sets the socket option SO_RCVBUF or similar, an int type. */
} SocketOptionEnum;

/** @enum SocketTypeEnum
 * Enumeration holding the type of sockets, to be used to setup low level
 * socket services, if needed.
 */
typedef enum
{
	SocketType_Default = 0,			/**< Default socket type, used for signaling and general purpose sockets. */
	SocketType_RTP,					/**< RTP socket type, used to convey that the socket will transport RTP media packets. */
	SocketType_RTCP					/**< RTCP socket type, used to convey that the socket will transport RTCP media control packets. */
} SocketTypeEnum;

/** @enum SocketMediaTypeEnum
 * Enumeration that differentiates the service that the socket will be used for.
 * This is particularly useful when assigning QoS directives to each socket.
 */
typedef enum
{
	SocketMediaType_Default = 0,	/**< Default socket media type, use best effort. */
	SocketMediaType_Audio,			/**< The socket will be used for audio data. */
	SocketMediaType_Video,			/**< The socket will be used for video data */
	SocketMediaType_Text			/**< The socket will be used for text data. */
} SocketMediaTypeEnum;

/** @enum SocketTLSVersionEnum
* Enumeration holding the type of TLS version.
*/
typedef enum
{
	SocketTLSVersion_Default = 0,	/**< Default version. The highest version which is mutually supported by the client and the server */
	SocketTLSVersion_TLSv1,			/**< TLS 1.x */
	SocketTLSVersion_TLSv1_0,		/**< TLS 1.0 */
	SocketTLSVersion_TLSv1_1,		/**< TLS 1.1 */
	SocketTLSVersion_TLSv1_2,		/**< TLS 1.2 */
} SocketTLSVersionEnum;

/** @struct SocketTLSSettingsStruct
* This structure is used by the Calling Component to provide all the
* required TLS parameters to the Socket Module via
* the DRVSocketCreate() function.
*/
typedef struct
{
	const u_char *pCiphers;       /**< Pointer to cipher list, if NULL the default/limited set is used. Optional, if NULL then default set used */
	BoolEnum bVerifyPeer;		/**< TRUE if peer certificate verification is required, FALSE otherwise. */
	const u_char *pCert;          /**< Path to CA certificate. Ignored if peer-verification is FALSE. MUST not be NULL if verification is TRUE. */
	u_int32 uNumOfFingerPrint;			/** < number of fingerprints present in the array */
	u_char **ppFingerPrint;			/**< array of the fingerprint from sdp attribute, peer-verification should be FALSE. */
	u_int32 connectionTimeoutMilli;	/**< The maximum time client would wait for the response from server during SSL connection . Optional, if 0 then default value is used */
	SocketTLSVersionEnum socketTLSVersion; /**< Indicates the version of TLS, currently not enforced and default version is recommended. */
} SocketTLSSettingsStruct;

/** @struct SocketCreateConfigStruct
 * This structure is used to provide configuration parameters to the
 * pal_SocketCreate() function.
 */
typedef struct
{
	ProtocolVersionEnum protocolVersion;	/**< The protocol version that the socket will support. */
	ProtocolTypeEnum protocolType;			/**< The protocol that the socket will support. */
	SocketTypeEnum socketType;				/**< The service that the socket will be used for, RTP or RTCP. */
	SocketMediaTypeEnum socketMediaType;	/**< The media service type that the socket should assume. */
	u_int32 bufferSize;						/**< The buffer size to use for allocating a receive buffer. */
	BoolEnum bEnableGlobalMutex;			/**< Set to Enum_TRUE if the Global Mutex should be locked before invoking the callback upon packet reception. */
	SocketTLSSettingsStruct *tlsSetting;	/**< Contains information of TLS. MUST be valid pointer for TLS type socket, otherwise ignored. */
} SocketCreateConfigStruct;

/** @struct SocketReceiveStruct
 * This is the structure for the received data and associated fields.
 */
typedef struct
{
	SOCKETHANDLE handle;		/**< Pointer to the Socket instance. */
	u_char *pReceiveData;		/**< Pointer to a byte buffer containing the received data. */
	u_int32 receiveLength;		/**< The length of the data received that the pCallbackFnData buffer points to. */
	const u_char *pFromIP;		/**< Points to an IP address of the host that sent the packet. */
	u_int16 fromPort;			/**< The port of the host that sent the packet. */
	u_int16 uTTL_HL;			/**< The Time To Live or Hop Limit that indicated in the IPv4 or IPv6 header. */
	u_int32 result;				/**< A result code indicating the validity of the received data or some other error. */
} SocketReceiveStruct;

/** @name Callback Functions
 * \n Callback functions must be implemented by the Calling Component and are
 * invoked in response to various actions and events. The callback functions
 * supported by this module include:
 */
/*@{*/

/**
 * This callback function will be triggered whenever data is received for
 * a socket. This function can only be called while a socket is successfully
 * open until the socket is closed. The function provides a Calling Component
 * with a pointer to a byte buffer containing the received data along with a
 * size indicating how much data in the buffer is valid.
 *
 * If the Calling Component does not set the remote host, this callback should
 * provide the actual host information in the pFromIP and fromPort parameters.
 * If the remote host was set, and its port was not set to 0 (zero), then these
 * parameters shouldn't be used (they may be NULL).
 *
 * In most cases, an implementation will call the module's error callback
 * function in case of detected errors. But in some cases an error may be
 * returned via this function, in which case the pCallbackFnData pointer
 * should be NULL, the receiveLength should be 0 (zero), and the result
 * code should not be KPALErrorNone. For general receive errors that might
 * be recoverable, the KPALDataReceiveError should be specified, which
 * indicates that some receive error occurred but the socket will still
 * wait for the next packet.
 *
 * This function won't be called if the Calling Component sets the callback
 * pointer to NULL. In these cases, the Calling Component might implement
 * a client that simply sends data but does not want to receive data.
 *
 * It is possible for UDP sockets to receive 0 (zero) length packets. These
 * packets will cause this callback to be invoked, and it is up to the
 * Calling Component whether to handle it or not.
 *
 * It is not safe to call the pal_SocketClose() or pal_SocketDeinit()
 * functions from within this callback due to the possibility of deadlock.
 *
 * @param[in] pCallbackFnData	Pointer to opaque data that the Calling
 *								Component will use. This pointer was provided
 *								by the Calling Component in the call to
 *								pal_SocketSetCallbacks(). This is typically
 *								used to point to the instance of the Calling
 *								Component that is utilizing the Socket
 *								Module. This pointer should not be freed by
 *								the Calling Component, because it will still
 *								be used for other expiration events.
 * @param[in] pReceive			Pointer to a structure for the received data
 *								and associated fields.
 * @return None.
 */
typedef void (*SocketReceiveCallback)
(
	void *pCallbackFnData,
	SocketReceiveStruct *pReceive
);

/**
 * This callback function will be triggered mainly for significant errors
 * detected by a socket, which would probably require that the socket be
 * closed (the socket maintained by the Socket Module might have already
 * been closed). This function can be called at almost any time while a
 * Socket Module is initialized, but will usually only be called after
 * a socket is successfully open and until the socket is closed.
 *
 * It is acceptable to call the pal_SocketClose() function from within this
 * callback. It is NOT safe to call the pal_SocketDeinit() function due to
 * the possibility of deadlock.
 *
 * @param[in] handle			Pointer to the Socket socket instance
 *								handle for which the data is received.
 *								If the pointer is NULL, the callback
 *								function should return without processing the
 *								event. This pointer should not be freed by the
 *								Calling Component.
 * @param[in] pCallbackFnData	Pointer to opaque data that the Calling
 *								Component will use. This pointer was provided
 *								by the Calling Component in the call to
 *								pal_SocketSetCallbacks(). This is typically
 *								used to point to the instance of the Calling
 *								Component that is utilizing the Socket
 *								Module. This pointer should not be freed by
 *								the Calling Component, because it will still
 *								be used for other expiration events.
 * @param[in] error				An error code indicating the type of error
 *								experienced. It will correspond to any of the
 *								error codes defined by EcrioPALErrors
 *								enumeration.
 * @return None.
 */
typedef void (*SocketErrorCallback)
(
	SOCKETHANDLE handle,
	void *pCallbackFnData,
	u_int32 error
);

/**
 * This callback function will be triggered whenever data is received for
 * a TCP server socket socket for the connection.
 * This function can only be called while a socket is successfully
 * open until the socket is closed. The function provides a Calling Component
 * with a pointer to remote IP address and a value of remote port.
 *
 * @param[in] listenHandle		Pointer to the Socket listening socket instance
 *								handle.PFM will search the callback function pointer by
 *								using this handle and let know to the upper layer about
 *								the newly accepted connection.
 *								If the pointer is NULL, the callback
 *								function should return without processing the
 *								event. This pointer should not be freed by the
 *								Calling Component.
 * @param[in] acceptHandle		Pointer to the Socket instance
 *								handle for which the data is received.
 *								If the pointer is NULL, the callback
 *								function should return without processing the
 *								event. This pointer should not be freed by the
 *								Calling Component.
 * @param[in] pCallbackFnData	Pointer to opaque data that the Calling
 *								Component will use. This pointer was provided
 *								by the Calling Component in the call to
 *								pal_SocketSetCallbacks(). This is typically
 *								used to point to the instance of the Calling
 *								Component that is utilizing the Socket
 *								Module. This pointer should not be freed by
 *								the Calling Component, because it will still
 *								be used for other expiration events.
 * @param[in] remoteIPVersion	The IP version (IPv4/IPv6)of the host that want
 *								to the connect in TCP server.
 * @param[in] pRemoteIP			Points to an IP address of the host that want
 *								to the connect in TCP server.
 * @param[in] remotePort		The port of the host that want
 *								to the connect in TCP server.
 * @param[out] pAccept			Points the information that calling component
 *								wish to accept this connection or not.
 *								If calling component sets the value TRUE,
 *								the incoming connection would be accepted.
 * @param[in] result			A result code indicating the validity of the
 *								received data or some other error. In general,
 *								if the value is equal to KPALErrorNone, the
 *								callback being issued is valid. It will
 *								correspond to any of the error codes defined
 *								by EcrioPALErrors enumeration.
 * @return None.
 */
typedef void (*SocketAcceptCallback)
(
	SOCKETHANDLE listenHandle,
	SOCKETHANDLE acceptHandle,
	void *pCallbackFnData,
	ProtocolVersionEnum remoteIPVersion,
	const u_char *pRemoteIP,
	u_int16 remotePort,
	BoolEnum *pAccept,
	u_int32 result
);

/*@}*/

/** @struct SocketCallbackFnStruct
 * This structure is used by the Calling Component to provide all the
 * required callback function pointers to the Socket Module via
 * the pal_SocketSetCallbacks() function for a created socket.
 */
typedef struct
{
	SocketReceiveCallback pReceiveCallbackFn;	/**< Pointer to a callback function to be called when packets are received. If this pointer is NULL, no packets will be received by the socket. */
	SocketErrorCallback pErrorCallbackFn;		/**< Pointer to a callback function to be called when a socket error is detected. If this pointer is NULL, errors won't be notified to the Calling Component. */
	SocketAcceptCallback pAcceptCallbackFn;		/**< Pointer to a callback function to be called when a incoming connection want to connect in a TCP server socket. If this pointer is NULL, connection won't be notified to the Calling Component. */
	void *pCallbackFnData;						/**< Pointer to opaque data that the Calling Component will use. This is sent with each callback notification. */
} SocketCallbackFnStruct;

/**
 * This function will create a socket, initialize it, and setup and allocate
 * other resources associated with the specific socket.
 *
 * The function returns a parameter which points to the created socket.
 *
 * @param[in] pal
 * @param[in] pConfig			Pointer to a configuration structure. Fields
 *								from the structure must be copied since the
 *								pointer may expire after the function returns.
 * @param[in] pFnStruct			Pointer to a CallbackFnStruct structure
 *								holding callback function pointers that the
 *								Calling Component wishes to use. This pointer
 *								must not be NULL, otherwise an error is
 *								returned.
 * @param[out] handle			Pointer to the socket handle that will be
 *								maintained until the socket is closed with a call to
 *								pal_SocketClose().
 *
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketCreate
(
	PALINSTANCE pal,
	SocketCreateConfigStruct *pConfig,
	SocketCallbackFnStruct *pFnStruct,
	SOCKETHANDLE *handle
);

/**
 * This function is used to set information for a local host, which includes
 * the IP address and port number. The Calling Component must know the host
 * information of the local host through some other means, or else the host can
 * be configured to be a "wildcard" in which case the socket subsystem would be
 * expected to assign specific host information when the socket is opened.
 *
 * If the socket is already open, the function will return an error. Otherwise,
 * the host information will be recorded by the implementation in preparation
 * for the call to pal_SocketOpen().
 *
 * The IP address can be a valid IP address or a wildcard, including NULL,
 * an empty string "", or the IP "0.0.0.0". If a wildcard, the implementation
 * should accept the IP address. When the pal_SocketOpen() function is
 * called, the socket subsystem should then dynamically assign a local
 * interface and IP address.
 *
 * The port number can be 0, in which case the implementation should
 * dynamically assign a valid port number when the pal_SocketOpen() function
 * is called.
 *
 * @param[in] handle			A Socket Module socket handle. If it is
 *								NULL, then the function will return an error.
 * @param[in] pLocalIP			Pointer to the local IP address to use. The
 *								address must be a null terminated C string.
 *								If the pointer is NULL, is an empty string, or
 *								is "0.0.0.0", the actual IP will be chosen by
 *								the underlying socket implementation when
 *								it is bound during the pal_SocketOpen()
 *								function.
 * @param[in] localPort			The local port number to use. If the local port
 *								is 0 (zero), the underlying socket
 *								implementation should choose the port to use
 *								dynamically.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketSetLocalHost
(
	SOCKETHANDLE handle,
	const u_char *pLocalIP,
	u_int16 localPort
);

/**
 * This function is used to set information for a remote host, which includes
 * the IP address and port number. The Calling Component must know the host
 * information of the remote host through some other means.
 *
 * If the socket is already open, the function will typically call the socket
 * API's connect() function to re-establish the default connection. Otherwise,
 * the host information will be recorded by the implementation in preparation
 * for the call to pal_SocketOpen().
 *
 * The IP address must be a valid IP address. The port number can be any value
 * in the 16-bit unsigned integer range, including 0. If 0 (zero) is specified
 * as the remote port, it implies that the socket will be able to receive data
 * from the specified IP address at any port. If the port is set to 0, the
 * pal_SocketSendData() function can not be used.
 *
 * Calling this function is optional. A Calling Component that didn't call this
 * function could only use the pal_SocketSendDataTo() function to send data
 * to a remote host.
 *
 * @param[in] handle			A Socket Module socket handle. If it is
 *								NULL, then the function will return an error.
 * @param[in] pRemoteIP			Pointer to the remote IP address to use. The
 *								address must be a null terminated C string.
 *								If the remote IP is NULL, an error will be
 *								returned.
 * @param[in] remotePort		The remote port number to use.
 *
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketSetRemoteHost
(
	SOCKETHANDLE handle,
	const u_char *pRemoteIP,
	u_int16 remotePort
);

/**
 * This function retrieves the local host information. The behavior can differ
 * based on whether the socket is open or not. If the socket is not open, the
 * local host information would be equivalent to that set with the call to
 * pal_SocketSetLocalHost(). If the socket is open, the information returned
 * should be read from the socket itself, possibly by using the standard socket
 * API's getsockname() function. This is useful because if wildcard parameters
 * were provided, the getsockname() function would return the actual IP address
 * and port that were chosen for the socket.
 *
 * If the pal_SocketSetLocalHost() has not yet been called, or failed, this
 * function would return an error.
 *
 * @param[in] handle			A Socket Module socket handle. If it is
 *								NULL, then the function will return an error.
 * @param[in] pLocalIP			Pointer to a buffer to copy the local IP to. If
 *								the pointer is NULL, an error is returned. The
 *								currently set local IP address will be copied
 *								to this buffer. Therefore, the buffer should be
 *								of adequate size to accommodate a dot-decimal
 *								IP address (at least 16 characters, including
 *								a null terminator).
 * @param[in] pLocalPort		Pointer to a location to copy the local port
 *								to. If the pointer is NULL, an error is
 *								returned. The currently set local port will be
 *								copied to this buffer.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketGetLocalHost
(
	SOCKETHANDLE handle,
	u_char *pLocalIP,
	u_int16 *pLocalPort
);

/**
 * This function retrieves the remote host information. The behavior can differ
 * based on whether the socket is open or not. If the socket is not open, the
 * remote host information would be equivalent to that set with the call to
 * pal_SocketSetRemoteHost(). If the socket is open, the information returned
 * should be read from the socket itself, possibly by using the standard socket
 * API's getpeername() function.
 *
 * If the pal_SocketSetRemoteHost() has not yet been called, or failed, this
 * function would return an error.
 *
 * @param[in] handle			A Socket Module socket handle. If it is
 *								NULL, then the function will return an error.
 * @param[in] pRemoteIP			Pointer to a buffer to copy the remote IP to.
 *								If the pointer is NULL, an error is returned.
 *								The currently set remote IP address will be
 *								copied to this buffer. Therefore, the buffer
 *								should be of adequate size to accommodate a
 *								dot-decimal IP address (at least 16 characters,
 *								including a null terminator).
 * @param[in] pRemotePort		Pointer to a location to copy the remote port
 *								to. If the pointer is NULL, an error is
 *								returned. The currently set remote port will be
 *								copied to this buffer.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketGetRemoteHost
(
	SOCKETHANDLE handle,
	u_char *pRemoteIP,
	u_int16 *pRemotePort
);

/**
 * This function will open a socket. The functions pal_SocketCreate(),
 * pal_SocketSetCallbacks(), and pal_SocketSetLocalHost() should have all
 * been called successfully before this function could succeed.
 * This function will typically call the standard socket API's bind() function,
 * which would bind the local host information to an interface and allow
 * packets to be sent. Likewise, if the remote host was set, the connect()
 * function could also be called. Even though UDP is connectionless, calling
 * connect() is still possible and could offer efficiency improvements.
 *
 * When a socket is bound, any wildcard settings for the local host will be
 * resolved.
 *
 * If the receive callback is not NULL, as set by the call to the
 * pal_SocketSetCallbacks() function, the socket should start waiting for
 * incoming packets. In a threaded implementation, this could involve
 * establishing a dedicated thread in which to call the blocking recv() and/or
 * recvfrom() functions. An implementation could choose any method available
 * as long as packets would be received and processed.
 *
 * @param[in] handle			A Socket Module socket handle. If it is
 *								NULL, then the function will return an error.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketOpen
(
	SOCKETHANDLE handle
);

/**
 * This function will open a socket. The functions pal_SocketCreate(),
 * pal_SocketSetCallbacks(), and pal_SocketSetLocalHost() should have all
 * been called successfully before this function could succeed.
 * This function will typically call the standard socket API's bind() function
 * only for TCP Server type socket,which would bind the local host information
 * to an interface and allow packets to be sent.
 * Likewise, if the remote host was set, the connect()
 * function could also be called. Even though UDP is connectionless, calling
 * connect() is still possible and could offer efficiency improvements.
 *
 * When a socket is bound, any wildcard settings for the local host will be
 * resolved.
 *
 * If the receive callback is not NULL, as set by the call to the
 * pal_SocketSetCallbacks() function, the socket should start waiting for
 * incoming packets. In a threaded implementation, this could involve
 * establishing a dedicated thread in which to call the blocking recv() and/or
 * recvfrom() functions. An implementation could choose any method available
 * as long as packets would be received and processed.
 *
 * @param[in] handle			A Socket Module socket handle. If it is
 *								NULL, then the function will return an error.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketOpenEx
(
		SOCKETHANDLE handle
);

/**
 * This function sends a data packet using the open socket. This function
 * can only be called if the remote host was set with a call to the
 * pal_SocketSetRemoteHost() function. Typically, the standard socket API's
 * send() function is used.
 *
 * This function is synchronous. However, even if the functions returns
 * successfully, there is no guarantee that the packet reached the remote
 * host, or if the packet even left the local host.
 *
 * @param[in] handle			A Socket Module socket handle. If it is
 *								NULL, then the function will return an error.
 * @param[in] pDataBuffer		Pointer to a buffer of byte data to send over
 *								the currently open socket. If the pointer is
 *								NULL, the dataLength parameter must be 0, or
 *								else an error is returned. The function will
 *								not free the memory pDataBuffer points to. The
 *								Calling Component is responsible for doing so
 *								upon return.
 * @param[in] dataLength		The length of data to be sent. This represents
 *								the length of the data pointed to by the
 *								pDataBuffer parameter and can be 0, whether or
 *								not the pDataBuffer is NULL.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketSendData
(
	SOCKETHANDLE handle,
	const u_char *pDataBuffer,
	u_int32 dataLength
);

/**
 * This function closes the socket that was created with a call to the
 * pal_SocketCreate() function and optionally opened with a call to the
 * pal_SocketOpen() function. The socket would typically be closed by
 * calling the standard socket API's closesocket() function. Then, the
 * function would cleanup all other resources allocated for the socket
 * instance.
 *
 * If the socket was opened, it will be closed with no further callback
 * functions being invoked. This function guarantees that all callback
 * functions already invoked before or during execution of this function call
 * will be completed before the function itself returns. Hence, it will wait
 * (i.e. block) for any outstanding callbacks to return before returning
 * itself.
 *
 * Although an implementation may offer some built-in protection, an
 * application should ensure that no other API is called for this socket after
 * this function has been called.
 *
 * @param[in] handle			Pointer to a Socket Module socket handle.
 *								If the pointer is NULL or if the instance
 *								handle is NULL, then the function will return
 *								an error. If it is not NULL, an attempt will be
 *								made to clean up resources and memory used by
 *								the instance. The instance handle will be set
 *								to NULL before returning.
 * @return KPALErrorNone is successful, otherwise a specific error code.
 */
u_int32 pal_SocketClose
(
	SOCKETHANDLE *handle
);

u_int32 pal_SocketSetNetID
(
	PALINSTANCE pal,
	u_int64 uNetID
);

u_int32 pal_SocketSetDeviceName
(
	PALINSTANCE pal,
	const char *pDeviceName
);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_SOCKET_H__ */
