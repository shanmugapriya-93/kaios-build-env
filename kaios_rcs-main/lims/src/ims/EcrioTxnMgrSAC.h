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

#ifndef _ECRIO_TXN_MGR_SAC_H_
#define _ECRIO_TXN_MGR_SAC_H_

#include "EcrioPAL.h"

/**Enumerated constant to indicate whether the channel is created over the port_uc or port_us.	*/
typedef enum
{
	ECRIO_TXNMGR_IPSEC_CHANNEL_TYPE_ENUM_None = 0,			/**<Default value.*/
	ECRIO_TXNMGR_IPSEC_CHANNEL_TYPE_ENUM_Client,			/**<The channel is created between port_uc and port_ps.*/
	ECRIO_TXNMGR_IPSEC_CHANNEL_TYPE_ENUM_Server			/**<The channel is created between port_us and port_pc.*/
}EcrioTxnMgrIPsecChannelTypeEnum;

/**Enumerated constant to indicate the current state of a particular IPsec.	*/
typedef enum
{
	ECRIO_TXNMGR_IPSEC_STATE_ENUM_None = 0,				/**<Default value.*/
	ECRIO_TXNMGR_IPSEC_STATE_ENUM_Temporary,				/**<An IPsec will be in this state after creation from 401 REGISTER and before receiving 200 REGISTER.*/
	ECRIO_TXNMGR_IPSEC_STATE_ENUM_Established,				/**<An IPsec will be in this state after receiving 200 REGISTER and before any receiving any other request from server over this IPsec.*/
	ECRIO_TXNMGR_IPSEC_STATE_ENUM_Active,					/**<An IPsec will be in this state after receiving any request from server over this IPsec.*/
	ECRIO_TXNMGR_IPSEC_STATE_ENUM_Expiring					/**<An IPsec will be in this state for 64*T1 after a new IPsec is established with the server.*/
}EcrioTxnMgrIPsecStateEnum;

/**This structure is used to describe the IPsec informations created over a channel.
*This structure is contained within EcrioSACSignalingCommunicationChannelInfoStruct.
*@brief used to describe the IPsec informations created over a channel.*/
typedef struct
{
	u_int32							uIPsecId; /**<The identifier of the IPsec.*/
	EcrioTxnMgrIPsecStateEnum			eIPsecState; /**<The current state of the IPsec.*/
	u_int32							uLocalPort_c; /**<The local client port of the IPsec associated with the channel.*/
	u_int32							uLocalPort_s; /**<The local server port of the IPsec associated with the channel.*/
	u_int32							uRemotePort_c; /**<The remote client port of the IPsec associated with the channel.*/
	u_int32							uRemotePort_s; /**<The remote server port of the IPsec associated with the channel.*/
}
EcrioTxnMgrChannelIPsecProtectionStruct;

typedef struct
{
	ProtocolTypeEnum eSocketType;							/**<The type of transport used for the channel (TCP/UDP etc).*/
	u_char *pLocalIp;										/**<Local IP address that the channel is bound on.*/
	u_int32	localPort;										/**<Local port that the channel is bound to.*/
	u_char *pRemoteIp;										/**<Remote IP address that the channel is used to access.*/
	u_int32	remotePort;										/**<Remote port that the channel sends to, or receives from.*/
	u_int32	remoteTLSPort;										/**<Remote port that the channel sends to, or receives from.*/
	u_int32	remoteClientPort;

	//TCP variables temporarily placed here for common access via channel structures.
	u_char *pTcpBuffer;										/**< Pointer to a buffer to hold TCP data while segments are collected for frames. */
	u_int32 tcpBufferSize;									/**< Total size of the allocated TCP buffer. */
	u_int32 tcpConsumedSize;								/**< Current size of the data in the TCP buffer. */

	//TLS variables temporarily placed here for common access via channel structures.
	u_char *pTlsBuffer;										/**< Pointer to a buffer to hold TLS data while segments are collected for frames. */
	u_int32 tlsBufferSize;									/**< Total size of the allocated TLS buffer. */
	u_int32 tlsConsumedSize;								/**< Current size of the data in the TLS buffer. */

	/*	The following memebrs are added for IPsec	*/
	BoolEnum bIPsecProtected;	/**<Flag to indicates whether channel is protected by IPsec or not.*/

	BoolEnum bIsProxyRouteEnabled;	/**< Conditional variable to use proxy routing or not. */
	u_char	 *pHttpConnectMsg;	/**< HTTP Connect message to be sent on connected socket. */
}
EcrioTxnMgrSignalingCommunicationChannelInfoStruct;

typedef struct
{
	u_int32	channelIndex;												/**<Unique identifier for the channel.*/
	SOCKETHANDLE hChannelHandle;
	EcrioTxnMgrSignalingCommunicationChannelInfoStruct *pChannelInfo;	/**<Detailed information on the channel.*/
}
EcrioTxnMgrSignalingCommunicationChannelStruct;

typedef struct
{
	u_int32	noOfChannels;												/**<Number of available channels.*/
	EcrioTxnMgrSignalingCommunicationChannelStruct *pChannels;			/**<Pointer to array of channel data structures.*/
}
EcrioTxnMgrSignalingCommunicationStruct;

/* Data structure to hold SAC related information */
typedef struct
{
	EcrioTxnMgrSignalingCommunicationStruct *pCommunicationStruct;		/* Pointer to the communication info  */
	/*	Required for setting the Index of the channel ID
	    through which the data would be sent to the NW.
	    This index MUST be one amongst the set in "pCommunicationsStruct" above.
	    This index would be set by the application via a new API (in this case the SUE).
	*/
	u_int32	sendingChannelIndex;
	u_int32	sendingTCPChannelIndex;
	u_int32	sendingTLSChannelIndex;
	u_int32	sendingIPsecUdpChannelIndex;
	u_int32	sendingIPsecTCPChannelIndex;
	u_int32	receivedOnChannelIndex;
	u_int32	remoteSendingPort;
	BoolEnum bIPsecEnabled;
} EcrioTxnMgrTransportStruct;

#endif /* _ECRIO_SIG_MGR_SAC_HANDLER_ */
