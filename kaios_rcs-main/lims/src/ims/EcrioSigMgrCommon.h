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

#ifndef _ECRIO_SIG_MGR_COMMON_H_
#define  _ECRIO_SIG_MGR_COMMON_H_

#include "EcrioPAL.h"
#include "EcrioSIPDataTypes.h"

#define _ENABLE_DEFAULT_CALL_HANDLER_CPM_
#define _ENABLE_DEFAULT_CALL_HANDLER_MOIP_

/*
 @def
Typedef Definition - START
*/

/** Signaling Manager handle*/
typedef void *SIGMGRHANDLE;

/** Signaling Session Handle */
//typedef void *SIGSESSIONHANDLE;

/** Signaling Dialog Handle */
typedef void *DIALOGHANDLE;

/*
@def
Signaling Error Codes - START
*/
/** No Error, API call successfull.*/
#define ECRIO_SIGMGR_NO_ERROR						(0x0000U)			/**< No Error, API call successfull.*/
/** Parameters passed to API are incomplete*/
#define ECRIO_SIGMGR_INSUFFICIENT_DATA				0x0001U
/** Passed timerX value is out of bounds */
#define ECRIO_SIGMGR_INVALID_TIMER_X				0x0003U
/** SigMgr doesn't support passed transport type. */
#define ECRIO_SIGMGR_TRANSPORT_UNSUPPORTED			0x0004U
/** SigMgr doesn't support passed structure type*/
#define ECRIO_SIGMGR_STRUCT_TYPE_UNDEFINED			0x0005U
/** SigMgr hasn't been initialized*/
#define ECRIO_SIGMGR_NOT_INITIALIZED				0x0006U
/** SigMgr can not allocate memory*/
#define ECRIO_SIGMGR_NO_MEMORY						0x0007U
/** BAD Signaling Manager Handle*/
#define ECRIO_SIGMGR_BAD_HANDLE						0x0008U
/** Signaling Manager Initialization failed*/
#define ECRIO_SIGMGR_INIT_FAILED					0x0009U
/** INVALID urischeme passed*/
#define ECRIO_SIGMGR_INVALID_URI_SCHEME				0x000aU
/** Signaling Manager UDP socket error */
#define ECRIO_SIG_MGR_SIG_SOCKET_ERROR				0x000bU
/** SIP registration failed*/
#define ECRIO_SIG_MGR_REGISTRATION_FAILED			0x000cU
/** Identifier not found*/
#define ECRIO_SIGMGR_ID_NOT_FOUND					0x000dU
/** SigMgr couldn't initialize Dlg Mgr.*/
#define ECRIO_SIGMGR_BAD_DIALOGMGR_HANDLE			0x000eU
/** SigMgr couldn't initialize TXN Mgr.*/
#define ECRIO_SIGMGR_BAD_TRANSACTIONMGR_HANDLE		0x000fU
/** Invite request failed*/
#define ECRIO_SIGMGR_INVITE_FAILED					0x0010U
/** SigMgr couldn't initialize IMS Lib. Mgr.*/
#define ECRIO_SIGMGR_BAD_IMSLIB_HANDLE				0x0011U
/** Transaction failed at TXN Mgr*/
#define	ECRIO_SIGMGR_TRANSACTION_FAILED				0x0012U
/** Retransmitted message received*/
#define ECRIO_SIGMGR_MSG_RETRANSMITTED				0x0013U
/** Error occurred while using IMS lib APIs*/
#define ECRIO_SIGMGR_IMS_LIB_ERROR					0x0014U
/** Error occurred while using TXN Mgr APIs*/
#define ECRIO_SIGMGR_TRANSACTIONMGR_ERROR			0x0015U
/** Error occurred while using Dialog Mgr APIs*/
#define ECRIO_SIGMGR_DIALOGMGR_ERROR				0x0016U
/**Dialog not found error*/
#define ECRIO_SIGMGR_DIALOG_NOT_EXISTS				0x0017U
/**Bad message received from the network*/
#define ECRIO_SIGMGR_BAD_MESSAGE_RECEIVED			0x0018U
/**Platform Manager timer error*/
#define ECRIO_SIGMGR_TIMER_ERROR					0x0019U
/**Unknown error occured*/
#define ECRIO_SIGMGR_UNKNOWN_ERROR					0x001aU
/**BYE request failed.*/
#define ECRIO_SIGMGR_BYE_FAILED						0x001bU
/** Registration is already in progress*/
#define ECRIO_SIGMGR_REG_ALREADY_IN_PROGRESS		0x001cU
/** Transaction not found*/
#define ECRIO_SIGMGR_TRANSACTION_NOT_FOUND			0x001dU
/** Unknown error happened at UAS side.*/
#define ECRIO_SIG_MGR_SERVER_INTERNAL_ERROR			0x001eU
/** Cancel request failed*/
#define ECRIO_SIGMGR_CANCEL_FAILED					0x001fU
/** error occurred while offer answer*/
#define ECRIO_SIGMGR_OFFER_ANSWER_ERROR				0x0020U
/** Invalid operation execution error*/
#define ECRIO_SIGMGR_INVALID_OPERATION				0x0021U
/** Invalid Session Handle*/
#define ECRIO_SIGMGR_INVALID_SESSION_HANDLE			0x0022U
/** AKA response generation failed*/
#define ECRIO_SIGMGR_AKA_FAILURE					0x0024U
/** AKA Clinet Server Sync failure*/
#define ECRIO_SIGMGR_AKA_SYNC_FAILURE				0x0025U
/** Invalid Data*/
#define ECRIO_SIGMGR_INVALID_ARGUMENTS				0x0026U
/** The request Forked*/
#define ECRIO_SIGMGR_REQUEST_FORKED					0x0027U
/** Received the Retransmitted Request*/
#define ECRIO_SIGMGR_REQUEST_RETRANSMITTED			0x0028U
/** Invalid request received.*/
#define ECRIO_SIGMGR_INVALID_REQUEST				0x0029U
/** Invite Terminated.*/
#define ECRIO_SIGMGR_INVITE_TERMINATED				0x002cU
/**SigMgr has been retried to initialize*/
#define ECRIO_SIGMGR_ALREADY_INITIALIZED			0x002dU
/**Parameter Not Found*/
#define ECRIO_SIGMGR_PARAMETER_NOT_FOUND			0x002eU
/**Usage not found*/
#define ECRIO_SIGMGR_DIALOG_USAGE_NOT_EXISTS		0x002fU
/** SUBSCRIBE / REFER request failed*/
#define ECRIO_SIGMGR_SUBSCRIPTION_FAILED			0x0030U
/** Data structure error */
#define ECRIO_SIGMGR_DS_ERROR						0x0031U
/** Header Value Not Found */
#define ECRIO_SIGMGR_HEADER_VAL_NOT_FOUND			0x0032U
/** Notify request / response failed*/
#define ECRIO_SIGMGR_NOTIFY_FAILED					0x0033U
/** Invalid data*/
#define ECRIO_SIGMGR_INVALID_DATA					0x0034U
/** Session already deleted*/
#define ECRIO_SIGMGR_SESSION_DELETED				0x0036U
/** Incorrect Association handle*/
#define ECRIO_SIGMGR_INVALID_ASSOCIATION_HANDLE		0x0037U
/** Authentication error*/
#define ECRIO_SIGMGR_AUTH_ERROR						0x0038U
/** Dialog related errors*/
#define ECRIO_SIGMGR_DIALOG_ERROR					0x0039U
/** Error processing custom URI*/
#define ECRIO_SIGMGR_CUSTOM_URI_ERROR				0x003aU
/** Loop Detected*/
#define ECRIO_SIGMGR_INVITE_LOOP_FOUND				0x003bU
/** Message request failed*/
#define ECRIO_SIGMGR_MESSAGE_FAILED					0x003cU

/**Custom Timer range error.*/
#define ECRIO_SIGMGR_CUSTOM_TIMER_RANGE_ERROR		0x003fU
/*	Modification for SUE	BEGIN	*/
#define ECRIO_SIGMGR_SAC_INVALID_CHANNEL			0x0040U
/*	Modification for SUE	END	*/
/** Capability to throw error if the length of outgoing SIP message is greater than the provisioned max-length */
#define ECRIO_SIGMGR_SIP_MSG_LENGTH_TOO_LARGE		0x0043U
/* End - Capability to throw error if the length of outgoing SIP message is greater than the provisioned max-length */

/* Checking Error to Support Send Info To handle Incoming Message Response Invalidate by SigMgr */
/**Unable to handle Incoming Message Response after successfully prsing of IMSLibrary.*/
#define ECRIO_SIGMGR_HANDLING_RECEIVED_RESPONSE_ERROR		0x0044U

#define ECRIO_SIGMGR_MORE_THEN_TWO_AUTH_RECEIVED			0x0045U		/* definition used to indicate that third authorization challenge is received by the client  */

#define ECRIO_SIGMGR_CRITCAL_ERROR_NEED_TO_EXIT				0x0046U

/** MAC Failure (MAC != XMAC) encountered during AKA RES computation */
#define ECRIO_SIGMGR_AKA_MAC_FAILURE						0x0048U
/** SQN not in correct range for AKA RES computation */
#define ECRIO_SIGMGR_AKA_SQN_FAILURE						0x0049U

/* UPDATE related failure*/
#define ECRIO_SIGMGR_UPDATE_FAILURE							0x004aU

#define ECRIO_SIGMGR_STRING_ERROR							0x004bU

/*  TLS channel error */
#define ECRIO_SIGMGR_TLS_CONNECTION_FAILURE					0x004cU
/**
*Error Codes - END
*/

/**
* @def
*Signaling Info Codes - START
*/
/** Register request has been successfully send */
#define ECRIO_SIGMGR_REGISTER_REQUEST					0x1000U
/** Register request has been successfully resend. Retransmission timer expired. */
#define ECRIO_SIGMGR_REGISTER_REQUEST_RESEND			0x1001U
/** Reregister request has been successfully send */
#define ECRIO_SIGMGR_REREGISTER_REQUEST					0x1002U
/** Reregister request has been successfully resend. Retransmission timer expired. */
#define ECRIO_SIGMGR_REREGISTER_REQUEST_RESEND			0x1003U
/** DeRegister request has been successfully send */
#define ECRIO_SIGMGR_DEREGISTER_REQUEST					0x1004U
/** DeReregister request has been successfully resend. Retransmission timer expired. */
#define ECRIO_SIGMGR_DEREGISTER_REQUEST_RESEND			0x1005U
/** Invite Request Sent Successfully*/
#define ECRIO_SIGMGR_INVITE_REQUEST						0x1006U
/** Invite Request Resent Successfully*/
#define ECRIO_SIGMGR_INVITE_REQUEST_RESEND				0x1007U
/** Response send for Invite request*/
#define ECRIO_SIGMGR_INVITE_RESPONSE_SEND				0x1008U
/** Response resend for Invite request*/
#define ECRIO_SIGMGR_INVITE_RESPONSE_RESEND				0x1009U
/** ACK send for INVITE*/
#define ECRIO_SIGMGR_ACK_SEND							0x100aU
/** ACK resend for INVITE*/
#define ECRIO_SIGMGR_ACK_RESEND							0x100bU
/** Bye send for INVITE*/
#define ECRIO_SIGMGR_BYE_SEND							0x100cU
/** Bye resend for INVITE*/
#define ECRIO_SIGMGR_BYE_RESEND							0x100dU
/** response to Bye send*/
#define ECRIO_SIGMGR_BYE_RESPONSE_SEND					0x100eU
/** response to Bye resend*/
#define ECRIO_SIGMGR_BYE_RESPONSE_RESEND				0x100fU
/** Prack send for Prov INVITE Resp */
#define ECRIO_SIGMGR_PRACK_SEND							0x1010U
/** Prack resend for Prov INVITE Resp */
#define ECRIO_SIGMGR_PRACK_RESEND						0x1011U
/** Prack response send succesfully*/
#define ECRIO_SIGMGR_PRACK_RESPONSE_SEND				0x1012U
/** Prack response resend succesfully*/
#define ECRIO_SIGMGR_PRACK_RESPONSE_RESEND				0x1013U
/** Subscribe send  */
#define ECRIO_SIGMGR_SUBSCRIBE_SEND						0x1014U
/** Subscribe resend */
#define ECRIO_SIGMGR_SUBSCRIBE_RESEND					0x1015U
/** Notify response send succesfully*/
#define ECRIO_SIGMGR_NOTIFY_RESPONSE_SEND				0x1016U
/** Notify response resend succesfully*/
#define ECRIO_SIGMGR_NOTIFY_RESPONSE_RESEND				0x1017U

/** Invalid context.*/
#define ECRIO_SIGMGR_INVALID_CONTEXT					0x1018U
/** Cancel request sent succesfully*/
#define ECRIO_SIGMGR_CANCEL_SEND						0x1019U
/** Cancel request resend succesfully*/
#define ECRIO_SIGMGR_CANCEL_RESEND						0x101aU
/** Cancel response send succesfully*/
#define ECRIO_SIGMGR_CANCEL_RESPONSE_SEND				0x101bU
/** Cancel response resend succesfully*/
#define ECRIO_SIGMGR_CANCEL_RESPONSE_RESEND				0x101cU
/** Reregistration timer expired*/
#define ECRIO_SIGMGR_RE_REG_TIMER_EXPIRES				0x101dU
/** NoOp Packet Sent Successfully */
#define ECRIO_SIGMGR_NOOP_PACKET_SEND					0x102eU

#define ECRIO_SIGMGR_CALL_REDIRECTED					0x102fU

/** Message Request Sent Successfully*/
#define ECRIO_SIGMGR_MESSAGE_REQUEST					0x1030U
/** Message Request Resent Successfully*/
#define ECRIO_SIGMGR_MESSAGE_REQUEST_RESEND				0x1031U
/** Response send for Message request*/
#define ECRIO_SIGMGR_MESSAGE_RESPONSE_SEND				0x1032U
/** Response resend for Message request*/
#define ECRIO_SIGMGR_MESSAGE_RESPONSE_RESEND			0x1033U

/**Registration expired.*/
#define ECRIO_SIGMGR_REGISTRATION_EXPIRED				0x1034U
/** Session refresh request Sent Successfully */
#define ECRIO_SIGMGR_SESSION_REFRESH_REQUEST_SEND		0x1037U
/** response to Termination in Progress State Bye Send*/
#define ECRIO_SIGMGR_TERMINATION_BYE_RESPONSE_SEND		0x103cU
/** response to Termination in Progress State Bye ReSend*/
#define ECRIO_SIGMGR_TERMINATION_BYE_RESPONSE_RESEND	0x103dU
/* End - Resolving BYE Issue - BUG#7427 */
/** Session refresh request Sent Successfully */
#define ECRIO_SIGMGR_SESSION_TERMINATION_TRYING			0x1041U
/** Registration timer expired */
#define ECRIO_SIGMGR_REGISTRATION_TIME_EXPIRED			0x1042U

/** Requst send for UPDATE request*/
#define ECRIO_SIGMGR_UPDATE_SEND						0x1043U
/** Requst resend for UPDATE request*/
#define ECRIO_SIGMGR_UPDATE_RESEND						0x1044U
/** response to Update send*/
#define ECRIO_SIGMGR_UPDATE_RESPONSE_SEND				0x1045U
/** response to Update resend*/
#define ECRIO_SIGMGR_UPDATE_RESPONSE_RESEND				0x1046U

/** Error response sent succesfully*/
#define ECRIO_SIGMGR_ERROR_RESPONSE_SEND				0x1047U
/** Error response resent succesfully*/
#define ECRIO_SIGMGR_ERROR_RESPONSE_RESEND				0x1048U

/** Options Request Sent Successfully*/
#define ECRIO_SIGMGR_OPTIONS_REQUEST					0x1049U
/** Options Request Resent Successfully*/
#define ECRIO_SIGMGR_OPTIONS_REQUEST_RESEND				0x1050U
/** Response send for Options request*/
#define ECRIO_SIGMGR_OPTIONS_RESPONSE_SEND				0x1051U
/** Response resend for Options request*/
#define ECRIO_SIGMGR_OPTIONS_RESPONSE_RESEND			0x1052U

/**
*Info Codes - END
*/

/**
* @def
*Signaling Warning Codes - START
*/
/** Retransmission timer expired and response to a register request hasn't arrived*/
#define	ECRIO_SIGMGR_TIMER_RE_EXPIRES_REGISTER				0x2000U
/** Retransmission timer expired and response to a INVITE request hasn't arrived*/
#define	ECRIO_SIGMGR_TIMER_RE_EXPIRES_INVITE				0x2001U
/** Retransmission timer expired and response to a BYE request hasn't arrived*/
#define	ECRIO_SIGMGR_TIMER_RE_EXPIRES_BYE					0x2002U
/** Retransmission timer expired for response to a INVITE and ACK request hasn't arrived*/
#define	ECRIO_SIGMGR_TIMER_RE_EXPIRES_INVITE_RESPONSE		0x2003U
/**
*Warning Codes - END
*/

/**
* @def
*Signaling status Codes - START
*/
/** Invalid status code*/
#define ECRIO_SIGMGR_INVALID_STATUS_CODE				0x0000U
/** Successful Final response has been received for a request*/
#define ECRIO_SIGMGR_RESPONSE_SUCCESS					0x3000U
/** Authenication response has been received for a request*/
#define ECRIO_SIGMGR_RESPONSE_AUTHENTICATION			0x3001U
/** Proxy Authenication response has been received for a request*/
#define ECRIO_SIGMGR_RESPONSE_PROXY_AUTHENTICATION		0x3002U
/** request failed */
#define ECRIO_SIGMGR_REQUEST_FAILED						0x3003U
/** 1xx response received for INVITE*/
#define ECRIO_SIGMGR_INVITE_IN_PROGRESS					0x3004U
/** 1xx response received for request*/
#define ECRIO_SIGMGR_REQUEST_IN_PROGRESS				0x3005U
/** 3xx response redirected*/
#define ECRIO_SIGMGR_RESPONSE_REDIRECTED				0x3006U
/** request failed due to Timeout */
#define ECRIO_SIGMGR_REQUEST_TIMEOUT					0x3007U
/** 423 Interval Too Brief Response received */
#define ECRIO_SIGMGR_REGISTER_INTERVAL_TOO_BRIEF		0x3008U
/**
*Signaling status Codes - END
*/

/**
* @def
* SIP Response Codes -START
*/
/** INVALID response code*/
#define ECRIO_SIGMGR_INVALID_RESPONSE_CODE						0U
/** Trying */
#define ECRIO_SIGMGR_RESPONSE_CODE_TRYING						100U
/** Ringing */
#define ECRIO_SIGMGR_RESPONSE_CODE_RINGING						180U
/** Call Is Being Forwarded */
#define ECRIO_SIGMGR_RESPONSE_CODE_CALL_FWD						181U
/** Queued */
#define ECRIO_SIGMGR_RESPONSE_CODE_QUEUED						182U
/** Session Progress */
#define ECRIO_SIGMGR_RESPONSE_CODE_SESSION_IN_PROGRESS			183U
/** early dialog terminated */
#define ECRIO_SIGMGR_RESPONSE_CODE_EARLY_DIALOG_TERMINATED		199U
/** OK */
#define ECRIO_SIGMGR_RESPONSE_CODE_OK							200U
/** Accepted */
#define ECRIO_SIGMGR_RESPONSE_CODE_ACCEPTED						202U
/** Multiple Choices */
#define ECRIO_SIGMGR_RESPONSE_CODE_MULTIPLE_CHOICES				300U
/** Moved Permanently*/
#define ECRIO_SIGMGR_RESPONSE_CODE_MOVED_PERMANENTLY			301U
/** Moved Temporarily */
#define ECRIO_SIGMGR_RESPONSE_CODE_MOVED_TEMPORARILY			302U
/** Use Proxy */
#define ECRIO_SIGMGR_RESPONSE_CODE_USE_PROXY					305U
/** Alternative Service */
#define ECRIO_SIGMGR_RESPONSE_CODE_ALTERNATIVE					380U
/** Bad Request */
#define ECRIO_SIGMGR_RESPONSE_CODE_BAD_REQUEST					400U
/** Unauthorized */
#define ECRIO_SIGMGR_RESPONSE_CODE_UNAUTHORIZED					401U
/** Payment Required */
#define ECRIO_SIGMGR_RESPONSE_CODE_PAYMENT_REQD					402U
/** Forbidden */
#define ECRIO_SIGMGR_RESPONSE_CODE_FORBIDDEN					403U
/** Not Found */
#define ECRIO_SIGMGR_RESPONSE_CODE_NOT_FOUND					404U
/** Method Not Allowed */
#define ECRIO_SIGMGR_RESPONSE_CODE_METHOD_NOT_ALLOWED			405U
/** Not Acceptable*/
#define ECRIO_SIGMGR_RESPONSE_CODE_NOT_ACCEPTABLE				406U
/** Proxy Authentication Required */
#define ECRIO_SIGMGR_RESPONSE_CODE_PROXY_AUTH_REQD				407U
/** Request Timeout */
#define ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TIMEOUT				408U
/** Gone */
#define ECRIO_SIGMGR_RESPONSE_CODE_GONE							410U
/** Conditional Request Failed */
#define ECRIO_SIGMGR_RESPONSE_CODE_COND_REQEST_FAILED			412U
/** Request Entity Too Large */
#define ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_ENTITY_TOO_LARGE		413U
/** Request-URI Too Long */
#define ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_URI_TOO_LONG			414U
/** Unsupported Media Type */
#define ECRIO_SIGMGR_RESPONSE_CODE_UNSUPPORTED_MEDIA_TYPE		415U
/** Unsupported URI Scheme */
#define ECRIO_SIGMGR_RESPONSE_CODE_UNSUPPORTED_URI_SCHEME		416U
/** Bad Extension */
#define ECRIO_SIGMGR_RESPONSE_CODE_BAD_EXTENSION				420U
/** Extension Required */
#define ECRIO_SIGMGR_RESPONSE_CODE_EXTENSION_REQD				421U
/** Session interval too small */
#define ECRIO_SIGMGR_RESPONSE_CODE_SESSION_INTERVAL_SMALL		422U
/** Interval Too Brief */
#define ECRIO_SIGMGR_RESPONSE_CODE_INTERVAL_TOO_BRIEF			423U
/** Temporarily Unavailable */
#define ECRIO_SIGMGR_RESPONSE_CODE_TEMPORARILY_UNAVAILABLE		480U
/** Call/Transaction Does Not Exist */
#define ECRIO_SIGMGR_RESPONSE_CODE_CALL_TXN_NOT_EXIST			481U
/** Loop Detected */
#define ECRIO_SIGMGR_RESPONSE_CODE_LOOP_DETECTED				482U
/** Too Many Hops*/
#define ECRIO_SIGMGR_RESPONSE_CODE_TOO_MANY_HOPS				483U
/** Address Incomplete*/
#define ECRIO_SIGMGR_RESPONSE_CODE_ADDRESS_INCOMPLETE			484U
/** Ambiguous*/
#define ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_AMBIGUOUS			485U
/** Busy Here */
#define ECRIO_SIGMGR_RESPONSE_CODE_BUSY_HERE					486U
/** Request Terminated*/
#define ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_TERMINATED			487U
/** Not Acceptable Here */
#define ECRIO_SIGMGR_RESPONSE_CODE_NOT_ACCEPTABLE_HERE			488U
/** Bad Event */
#define ECRIO_SIGMGR_RESPONSE_CODE_BAD_EVENT					489U
/** Request Pending*/
#define ECRIO_SIGMGR_RESPONSE_CODE_REQUEST_PENDING				491U
/** Undecipherable*/
#define ECRIO_SIGMGR_RESPONSE_CODE_UNDECIPHERABLE				493U
/** Server Internal Error*/
#define ECRIO_SIGMGR_RESPONSE_CODE_SERVER_INTERNAL_ERROR		500U
/** Not Implemented*/
#define ECRIO_SIGMGR_RESPONSE_CODE_NOT_IMPLEMENTED				501U
/** Bad Gateway*/
#define ECRIO_SIGMGR_RESPONSE_CODE_BAD_GATEWAY					502U
/** Service Unavailable*/
#define ECRIO_SIGMGR_RESPONSE_CODE_SERVICE_UNAVAILABLE			503U
/** Server Time-out*/
#define ECRIO_SIGMGR_RESPONSE_CODE_SERVER_TIMEOUT				504U
/** Version Not Supported */
#define ECRIO_SIGMGR_RESPONSE_CODE_VERSION_NOT_SUPPORTED		505U
/** Message Too Large */
#define ECRIO_SIGMGR_RESPONSE_CODE_MESSAGE_TOO_LARGE			513U
/* Support for precondition - start */
/** Precondition failure */
#define ECRIO_SIGMGR_RESPONSE_CODE_PRECONDITION_FAILURE			580U
/* Support for precondition - end */
/** Busy Everywhere */
#define ECRIO_SIGMGR_RESPONSE_CODE_BUSY_EVERYWHERE				600U
/** Decline */
#define ECRIO_SIGMGR_RESPONSE_CODE_DECLINE						603U
/** Does Not Exist Anywhere */
#define ECRIO_SIGMGR_RESPONSE_CODE_NOT_EXIST_ANYWHERE			604U
/** Not Acceptable */
#define ECRIO_SIGMGR_RESPONSE_CODE_NOT_ACCEPTABLE_EVERYWHERE	606U
/** Response Code shall be below this value */
#define ECRIO_SIGMGR_RESPONSE_CODE_MAX							700U
/**
* SIP Response Codes - END
*/

/**
* @def
* SIP Response Reason Phrase - START
*/
/** Trying */
#define ECRIO_SIGMGR_RESPONSE_TRYING									(u_char *)"Trying"
/** Ringing */
#define ECRIO_SIGMGR_RESPONSE_RINGING									(u_char *)"Ringing"
/** Call Is Being Forwarded */
#define ECRIO_SIGMGR_RESPONSE_CALL_FWD									(u_char *)"Call Is Being Forwarded"
/** Queued */
#define ECRIO_SIGMGR_RESPONSE_QUEUED									(u_char *)"Queued"
/** Session Progress */
#define ECRIO_SIGMGR_RESPONSE_SESSION_IN_PROGRESS						(u_char *)"Session Progress"
/** OK */
#define ECRIO_SIGMGR_RESPONSE_OK										(u_char *)"OK"
/** ACCEPTED */
#define ECRIO_SIGMGR_RESPONSE_ACCEPTED									(u_char *)"Accepted"
/** Multiple Choices */
#define ECRIO_SIGMGR_RESPONSE_MULTIPLE_CHOICES							(u_char *)"Multiple Choices"
/** Moved Permanently*/
#define ECRIO_SIGMGR_RESPONSE_MOVED_PERMANENTLY							(u_char *)"Moved Permanently"
/** Moved Temporarily */
#define ECRIO_SIGMGR_RESPONSE_MOVED_TEMPORARILY							(u_char *)"Moved Temporarily"
/** Use Proxy */
#define ECRIO_SIGMGR_RESPONSE_USE_PROXY									(u_char *)"Use Proxy"
/** Alternative Service */
#define ECRIO_SIGMGR_RESPONSE_ALTERNATIVE								(u_char *)"Alternative Service"
/** Bad Request */
#define ECRIO_SIGMGR_RESPONSE_BAD_REQUEST								(u_char *)"Bad Request"
/** Unauthorized */
#define ECRIO_SIGMGR_RESPONSE_UNAUTHORIZED								(u_char *)"Unauthorized"
/** Payment Required */
#define ECRIO_SIGMGR_RESPONSE_PAYMENT_REQD								(u_char *)"Payment Required"
/** Forbidden */
#define ECRIO_SIGMGR_RESPONSE_FORBIDDEN									(u_char *)"Forbidden"
/** Not Found */
#define ECRIO_SIGMGR_RESPONSE_NOT_FOUND									(u_char *)"Not Found"
/** Method Not Allowed */
#define ECRIO_SIGMGR_RESPONSE_METHOD_NOT_ALLOWED						(u_char *)"Method Not Allowed"
/** Not Acceptable*/
#define ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE							(u_char *)"Not Acceptable"
/** Proxy Authentication Required */
#define ECRIO_SIGMGR_RESPONSE_PROXY_AUTH_REQD							(u_char *)"Proxy Authentication Required"
/** Request Timeout */
#define ECRIO_SIGMGR_RESPONSE_REQUEST_TIMEOUT							(u_char *)"Request Timeout"
/** Gone */
#define ECRIO_SIGMGR_RESPONSE_GONE										(u_char *)"Gone"
/** Request Entity Too Large */
#define ECRIO_SIGMGR_RESPONSE_REQUEST_ENTITY_TOO_LARGE					(u_char *)"Request Entity Too Large"
/** Request-URI Too Long */
#define ECRIO_SIGMGR_RESPONSE_REQUEST_URI_TOO_LONG						(u_char *)"Request-URI Too Long"
/** Unsupported Media Type */
#define ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_MEDIA_TYPE					(u_char *)"Unsupported Media Type"
/** Unsupported URI Scheme */
#define ECRIO_SIGMGR_RESPONSE_UNSUPPORTED_URI_SCHEME					(u_char *)"Unsupported URI Scheme"
/** Bad Extension */
#define ECRIO_SIGMGR_RESPONSE_BAD_EXTENSION								(u_char *)"Bad Extension"
/** Extension Required */
#define ECRIO_SIGMGR_RESPONSE_EXTENSION_REQD							(u_char *)"ExtensionRequired"
/** Interval Too Brief */
#define ECRIO_SIGMGR_RESPONSE_INTERVAL_TOO_BRIEF						(u_char *)"Interval Too Brief"
/** Temporarily Unavailable */
#define ECRIO_SIGMGR_RESPONSE_TEMPORARILY_UNAVAILABLE					(u_char *)"Temporarily Unavailable"
/** Call/Transaction Does Not Exist */
#define ECRIO_SIGMGR_RESPONSE_CALL_TXN_NOT_EXIST						(u_char *)"Call/Transaction Does Not Exist"
/** Subscription Does not exist*/
#define ECRIO_SIGMGR_RESPONSE_SUBSCRIPTION_NOT_EXIST					(u_char *)"Subscription Does Not Exist"
/** Call Does Not Exist */
#define ECRIO_SIGMGR_RESPONSE_CALL_NOT_EXIST							(u_char *)"Call Does Not Exist"
/** Loop Detected */
#define ECRIO_SIGMGR_RESPONSE_LOOP_DETECTED								(u_char *)"Loop Detected"
/** Too Many Hops*/
#define ECRIO_SIGMGR_RESPONSE_TOO_MANY_HOPS								(u_char *)"Too Many Hops"
/** Address Incomplete*/
#define ECRIO_SIGMGR_RESPONSE_ADDRESS_INCOMPLETE						(u_char *)"Address Incomplete"
/** Ambiguous*/
#define ECRIO_SIGMGR_RESPONSE_REQUEST_AMBIGUOUS							(u_char *)"Ambiguous"
/** Busy Here */
#define ECRIO_SIGMGR_RESPONSE_BUSY_HERE									(u_char *)"Busy Here"
/** Busy Here */
#define ECRIO_SIGMGR_RESPONSE_BUSY_HERE_REJECTED_BY_USER				(u_char *)"Call Rejected By User"
/** Busy Here */
#define ECRIO_SIGMGR_RESPONSE_BUSY_HERE_ESTABLISHING_ANOTHER_CALL		(u_char *)"Establishing Another Call"
/** Busy Here */
#define ECRIO_SIGMGR_RESPONSE_BUSY_HERE_ON_TWO_CALLS					(u_char *)"Already On Two Calls"
/** Busy Here */
#define ECRIO_SIGMGR_RESPONSE_BUSY_HERE_NO_ANSWER						(u_char *)"No Answer"
/** Request Terminated*/
#define ECRIO_SIGMGR_RESPONSE_REQUEST_TERMINATED						(u_char *)"Request Terminated"
/** Not Acceptable Here */
#define ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE_HERE						(u_char *)"Not Acceptable Here"

#define ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE_HERE_MOVED_TO_EHRPD		(u_char *)"Moved to eHRPD"

#define ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE_HERE_VOLTE_OFF				(u_char *)"VoLTE setting Off"

/** Bad Event */
#define ECRIO_SIGMGR_RESPONSE_BAD_EVENT									(u_char *)"Bad Event"
/** Request Pending*/
#define ECRIO_SIGMGR_RESPONSE_REQUEST_PENDING							(u_char *)"Request Pending"
/** Undecipherable*/
#define ECRIO_SIGMGR_RESPONSE_UNDECIPHERABLE							(u_char *)"Undecipherable"
/** Server Internal Error*/
#define ECRIO_SIGMGR_RESPONSE_SERVER_INTERNAL_ERROR						(u_char *)"Server Internal Error"
/** Not Implemented*/
#define ECRIO_SIGMGR_RESPONSE_NOT_IMPLEMENTED							(u_char *)"Not Implemented"
/** Bad Gateway*/
#define ECRIO_SIGMGR_RESPONSE_BAD_GATEWAY								(u_char *)"Bad Gateway"
/** Service Unavailable*/
#define ECRIO_SIGMGR_RESPONSE_SERVICE_UNAVAILABLE						(u_char *)"Service Unavailable"
/** Server Time-out*/
#define ECRIO_SIGMGR_RESPONSE_SERVER_TIMEOUT							(u_char *)"Server Time-out"
/** Version Not Supported */
#define ECRIO_SIGMGR_RESPONSE_VERSION_NOT_SUPPORTED						(u_char *)"Version Not Supported"
/** Message Too Large */
#define ECRIO_SIGMGR_RESPONSE_MESSAGE_TOO_LARGE							(u_char *)"Message Too Large"
/** Busy Everywhere */
#define ECRIO_SIGMGR_RESPONSE_BUSY_EVERYWHERE							(u_char *)"Busy Everywhere"
/** Decline */
#define ECRIO_SIGMGR_RESPONSE_DECLINE									(u_char *)"Decline"
/** Does Not Exist Anywhere */
#define ECRIO_SIGMGR_RESPONSE_NOT_EXIST_ANYWHERE						(u_char *)"Does Not Exist Anywhere"
/** Not Acceptable */
#define ECRIO_SIGMGR_RESPONSE_NOT_ACCEPTABLE_EVERYWHERE					(u_char *)"Not Acceptable"
/**Session Interval Too Small for 422 response */
#define ECRIO_SIGMGR_RESPONSE_SESSION_INTERVAL_SMALL					(u_char *)"Session Interval Too Small"
/** IMS Core Outage*/
#define ECRIO_SIGMGR_RESPONSE_IMS_CORE_OUTAGE							(u_char *)"Service Unavailable: IMS Core Outage"
/** Coced Not Supported*/
#define ECRIO_SIGMGR_RESPONSE_CODEC_NOT_SUPPORTED						(u_char *)"Codec Not Supported"
/** On eHRPD*/
#define ECRIO_SIGMGR_RESPONSE_ON_EHRPD									(u_char *)"On eHRPD"
/**
* SIP Response Reason Phrase - END
*/

#define ECRIO_SIGMGR_STR_RESPONSE_CODE_MOVED_TEMPORARILY				"302"

/**
* @def
* String Constants - START
*/
/** Content type SDP */
#define ECRIO_SIGMGR_CONTENTTYPE_SDP									"application/sdp"
#define ECRIO_SIGMGR_CONTENTTYPE_REG_INFO_XML							"application/reginfo+xml"
/** Content type Multipart */
#define ECRIO_SIGMGR_CONTENTTYPE_MULTIPART								"multipart/mixed"
/** Content Boundary Value in case of Multipart Message bodies */
#define ECRIO_SIGMGR_CONTENT_BOUNDARY_VALUE								"gemrockgemrock"
/** Content Disposition -  Session */
#define ECRIO_SIGMGR_CONTENT_DISPOSITION_SESSION						"session"
/** expires parameter*/
#define ECRIO_SIG_MGR_EXPIRES_PARAM										"expires"
/** qValue parameter*/
#define ECRIO_SIG_MGR_Q_PARAM											"q"
/** transport param*/
#define ECRIO_SIG_MGR_TRANSPORT_PARAM									"transport"
/** PRACK extension*/
#define ECRIO_SIG_MGR_PRACK_EXTENSION									"100rel"
/** Precondition extension*/
#define ECRIO_SIG_MGR_PRECONDITION_EXTENSION							"precondition"
/** id parameter for Event Header*/
#define ECRIO_SIG_MGR_ID_PARAM											"id"
/** Subscription State - Active*/
#define ECRIO_SIG_MGR_SUBSCRPTION_STATE_ACTIVE							"active"
/** Subscription State - Pending*/
#define ECRIO_SIG_MGR_SUBSCRPTION_STATE_PENDING							"pending"
/** Subscription State - Terminated*/
#define ECRIO_SIG_MGR_SUBSCRPTION_STATE_TERMINATED						"terminated"
/** reason parameter*/
#define ECRIO_SIG_MGR_REASON_PARAM										"reason"
/** Session timer support */
#define ECRIO_SIG_MGR_SESSION_TIMER_EXTENSION							"timer"
/** Session timer support */
#define ECRIO_SIG_MGR_SESSION_PATH_EXTENSION							"path"
#define ECRIO_SIG_MGR_SESSION_EVENT_VALUE								"reg"
#define ECRIO_SIG_MGR_SESSION_REFRESHER_PARAM							"refresher"
/** Session timer support */
#define ECRIO_SIG_MGR_SESSION_REFRESHER_UAC								"uac"
/** Session timer support */
#define ECRIO_SIG_MGR_SESSION_REFRESHER_UAS								"uas"
/* reason param strings for subscription states terminated*/
/** timeout parameter*/
#define ECRIO_SIG_MGR_REASON_TIMEOUT									"timeout"
/** to-tag param*/
#define ECRIO_SIG_MGR_TO_TAG											"to-tag"
/** from-tag param*/
#define ECRIO_SIG_MGR_FROM_TAG											"from-tag"
/** Early-Only Flag*/
#define ECRIO_SIG_MGR_EARLY_ONLY_FLAG									"early-only"

/** Content type sipfrag */
#define ECRIO_SIGMGR_CONTENTTYPE_MESSAGE_3GPP2_SMS						"application/vnd.3gpp2.sms"

/** Content type sipfrag */
#define ECRIO_SIGMGR_CONTENTTYPE_MESSAGE_3GPP_SMS						"application/vnd.3gpp.sms"

#define ECRIO_SIGMGR_REQUEST_DISPOSITION_VALUE_NO_FORK					"no-fork"
#define ECRIO_SIGMGR_SUPPORTED_VALUE_PATH								"path"


#define ECRIO_SIGMGR_GSMA_RCS_TELEPHONY_FEATURE_TAG_NAME_VALUE_STRING	(u_char *)"+g.gsma.rcs.telephony=\"cs,volte\""

/** Reason header value */

#define ECRIO_SIG_MGR_BYE_REASON_PROTOCOL								(u_char *)"SIP"
#define ECRIO_SIG_MGR_BYE_REASON_CAUSE									(u_char *)"cause"
#define ECRIO_SIG_MGR_BYE_REASON_CAUSE_VAL								(u_char *)"200"
#define ECRIO_SIG_MGR_BYE_REASON_TEXT									(u_char *)"text"
#define ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_MOIP							(u_char *)"\"User Triggered\""
#define ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_CPM							(u_char *)"\"Call completed\""
#define ECRIO_SIG_MGR_BYE_REASON_TEXT_VAL_NEW_DIALOG					(u_char *)"\"New Dialog Established\""

/** Maximum length to send the data through UDP, otherwise TCP*/
// #define MAX_SIP_LEN_OVER_UDP									1300U
#define MAX_SIP_LEN_OVER_UDP											1300U

/* End - Sending SIP Request through UDP if previous attempt failed in TCP. */
/**Maximum length of sending SIP request through UDP if previous attempt failed in TCP.*/
#define MAX_SIP_MAX_LEN_OVER_UDP										60000U
/* End - Sending SIP Request through UDP if previous attempt failed in TCP. */

#define  ECRIO_SIGMGR_MAX_SESSIONS										3

#define ECRIO_SIGMGR_BUFFER_SIZE_10									10
#define ECRIO_SIGMGR_BUFFER_SIZE_32									32
#define ECRIO_SIGMGR_BUFFER_SIZE_64									64

/**
* String Constants - END
*/

/**
* @def
*Enumerated Constant Definition - START
*/
/** Enumeration of constants that is used to specify the Signaling Mgr Information types */
/** Signaling Mgr communicates to the upper layer three types of
*information representing its seriousness.
*/
typedef enum
{
	/** Enumeration that is used to indicate the notification as Information level */
	EcrioSigMgrInformation = 0,
	/**Enumeration that is used to indicate the notification as warning level. */
	EcrioSigMgrWarning,
	/** Enumeration that is used to indicate the notification as error level.*/
	EcrioSigMgrError
} EcrioSigMgrInfoTypeEnum;

/** Signaling Mgr request / response types */
typedef enum
{
	/** Identifies a message as invalid message*/
	EcrioSigMgrMessageNone = 0,
	/** Identifies a message as register request*/
	EcrioSigMgrRegisterRequest,
	/** Identifies a message as register response*/
	EcrioSigMgrRegisterReponse,
	/** Identifies a message as deregister request*/
	EcrioSigMgrDeRegisterRequest,
	/** Identifies a message as deregister response*/
	EcrioSigMgrDeRegisterReponse,
	/** Identifies a message as message response*/
	EcrioSigMgrInstantMessageResponse,
	/** Identifies a message as publish response*/
	EcrioSigMgrPublishResponse,
	/** Identifies a message as message request*/
	EcrioSigMgrInstantMessageRequestNotification,
	EcrioSigMgrInviteRequestNotification,
	EcrioSigMgrInviteResponse,
	EcrioSigMgrReInviteRequestNotification,
	EcrioSigMgrReInviteResponse,
	EcrioSigMgrAckRequestNotifiction,
	EcrioSigMgrAckRequest,
	EcrioSigMgrByeRequestNotification,
	EcrioSigMgrByeResponseNotification,
	EcrioSigMgrCancelRequestNotification,
	EcrioSigMgrCancelResponseNotification,
	EcrioSigMgrPrackRequestNotification,
	EcrioSigMgrPrackResponseNotification,
	EcrioSigMgrSubscribeResponseNotification,
	EcrioSigMgrReferResponseNotification,
	EcrioSigMgrNotifyRequestNotification,
	EcrioSigMgrDeleteSessionNotification,
	EcrioSigMgrUpdateRequestNotification,
	EcrioSigMgrUpdateResponseNotification,
	EcrioSigMgrSessionRefreshNotification,
	EcrioSigMgrIPSecSecurityServerNotification,
	EcrioSigMgrOptionsRequestNotification,
	EcrioSigMgrOptionsResponseNotification
} EcrioSigMgrMessageCmdEnum;

/** Enumerated constants used to initialize and de initialize the engine structures */
typedef enum
{
	/** Enumerated constant that indicates an invalid structure type. */
	EcrioSigMgrStructType_invalid = -1,
	/** Enumerated constant that is used for initializing parameters. */
	EcrioSigMgrStructType_InitStruct = 0,
	/** Enumerated constant that represents structure - EcrioSigMgrSignalingInfoStruct */
	EcrioSigMgrStructType_SignalingInfo,
	/** Enumerated constant that represents structure - EcrioSigMgrEngineCallbackStruct */
	EcrioSigMgrStructType_EngineCallback,
	/** Enumerated constant that represents structure - EcrioSigMgrInfoStruct */
	EcrioSigMgrStructType_Info,
	/** Enumerated constant that represents structure - EcrioSigMgrMessageStruct */
	EcrioSigMgrStructType_Message,
	/** Enumerated constant that represents structure - EcrioSigMgrIPAddrStruct */
	EcrioSigMgrStructType_IPAddr,
	/**Enumerated constant that represents structure - EcrioSigMgrParamStruct */
	EcrioSigMgrStructType_Params,
	/**Enumerated constant that represents structure -  EcrioSigMgrSipURIStruct */
	EcrioSigMgrStructType_SipURI,
	/** Enumerated constant that represents structure - EcrioSigMgrUriStruct */
	EcrioSigMgrStructType_Uri,
	/** Enumerated constant that represents structure - EcrioSigMgrNameAddrStruct */
	EcrioSigMgrStructType_NameAddr,
	/** Enumerated constant that represents structure - EcrioSigMgrNameAddrWithParamsStruct */
	EcrioSigMgrStructType_NameAddrWithParams,
	/** Enumerated constant that represents structure - EcrioSigMgrHeaderValueStruct */
	EcrioSigMgrStructType_HeaderValues,
	/** Enumerated constant that represents structure - EcrioSigMgrHeaderStruct */
	EcrioSigMgrStructType_Header,
	/** Enumerated constant that represents structure - EcrioSigMgrConfiguredHeaderStruct */
	EcrioSigMgrStructType_ConfiguredHeader,
	/** Enumerated constant that represents structure - EcrioSigMgrHeaderListStruct */
	EcrioSigMgrStructType_HeaderList,
	/** Enumerated constant that represents structure - EcrioSigMgrMandatoryHeaderStruct */
	EcrioSigMgrStructType_MandatoryHeaders,
	/** Enumerated constant that represents structure - EcrioSigMgrRouteStruct */
	EcrioSigMgrStructType_Route,
	/** Enumerated constant that represents structure - EcrioSigMgrContactStruct */
	EcrioSigMgrStructType_Contact,
	/**Enumerated constant that represents structure - EcrioSigMgrViaStruct */
	EcrioSigMgrStructType_Via,
	/** Enumerated constant that represents structure - EcrioSigMgrMimeHeaderStruct */
	EcrioSigMgrStructType_MimeHeader,
	/** Enumerated constant that represents structure - EcrioSigMgrMessageBodyStruct */
	EcrioSigMgrStructType_MessageBody,
	/** Enumerated constant that represents structure - EcrioSigMgrAuthorizationStruct */
	EcrioSigMgrStructType_Authorization,
	/** Enumerated constant that represents structure - EcrioSigMgrAuthenticationStruct */
	EcrioSigMgrStructType_Authentication,
	/** Enumerated constant that represents structure - EcrioSigMgrUnknownMessageBodyStruct.*/
	EcrioSigMgrStructType_UnknownMessageBody,

	/*	Support for Authorization header in all request	start	*/
	/**Enumerated constant that represents structure - EcrioSigMgrSharedCredentialsStruct*/
	EcrioSigMgrStructType_SharedCredentials,
	/*	Support for Authorization header in all request	end	*/

	/** Enumerated constant that represents the number of structure types.*/
	EcrioSigMgrStructType_Num,
	/** Enumerated constant that represents the Change parameters structure. */
	EcrioSigMgrStructType_SignalingChangeParamInfo,
	/**Enumerated constant that represents structure - EcrioSigMgrNetworkInfoStruct.*/
	EcrioSigMgrStructType_NetworkInfoStruct,
	/** Enumerated constant that represents structure - EcrioSigMgrAuthenticationStruct */
	EcrioSigMgrStructType_AKA,
	EcrioSigMgrStructType_SipMessage
} EcrioSigMgrStructTypeEnum;

/** Enumeration of constants that is used to describe the SIP method.*/
typedef enum
{
	/** Enumerated constant that represents the Invalid Method None*/
	EcrioSigMgrMethodNone = 0,
	/**Enumerated constant that represents the REGISTER Method*/
	EcrioSigMgrMethodRegister,
	/**Enumerated constant that represents the MESSAGE method*/
	EcrioSigMgrMethodMessage
} EcrioSigMgrMethodTypeEnum;

/**
*Enumeration of constants that is used to describe the telephone subscriber type.
*/
typedef enum
{
	/** Enumerated constant that represents the Invalid Subscriber*/
	EcrioSigMgrTelSubscriberNone = 0,
	/** Enumerated constant that represents the Global Subscriber*/
	EcrioSigMgrTelSubscriberGlobal,
	/** Enumerated constant that represents the Local Subscriber*/
	EcrioSigMgrTelSubscriberLocal
} EcrioSigMgrTelSubscriberEnum;

/**
*Enumeration of constants that is used to describe the Tel parameter type.
*/
typedef enum
{
	/** Enumerated constant that represents the Invalid Tel parameter.*/
	EcrioSigMgrTelParNone = 0,
	/** Enumerated constant that represents the Parameter*/
	EcrioSigMgrTelParParameter,
	/**Enumerated constant that represents the Extension*/
	EcrioSigMgrTelParExtension,
	/**Enumerated constant that represents the Isdn Sub Address*/
	EcrioSigMgrTelParIsdnSubAddress
} EcrioSigMgrTelParEnum;

/**
*Enumeration of constants that is used to describe the Tel context type.
*/
typedef enum
{
	/** Enumerated constant that represents the Invalid Tel context.*/
	EcrioSigMgrTelContextNone = 0,
	/**Enumerated constant that represents the Context represnts Domain Name*/
	EcrioSigMgrTelContextDomainName,
	/**Enumerated constant that represents the Context represents Global Phone digits*/
	EcrioSigMgrTelContextGlobalNumDigits
} EcrioSigMgrTelContextEnum;

/** Enumeration of constants that is used to describe the URI Schemes - to be used by Signaling Manager while constructing address of a record.*/
typedef enum
{
	/**Enumerated constant that represents the URI scheme None.*/
	EcrioSigMgrURISchemeNone = 0,
	/** Enumerated constant that represents the SIP URI scheme. */
	EcrioSigMgrURISchemeSIP,
	/** Enumerated constant that represents the TEL URI scheme.*/
	EcrioSigMgrURISchemeTEL,
	/** Enumerated constant that represents the Custom URI scheme. */
	EcrioSigMgrURISchemeCustom
} EcrioSigMgrURISchemesEnum;

/** Enumeration of constants that is used to describe the signaling manager transport, which will be used by the signaling manager while creating socket.*/
typedef enum
{
	/**Enumerated constant that represents the  UDP transport to be used*/
	EcrioSigMgrTransportUDP = 0,
	/**Enumerated constant that represents the  TCP transport to be used*/
	EcrioSigMgrTransportTCP = 1,
	/**Enumerated constant that represents the  TLS transport to be used*/
	EcrioSigMgrTransportTLS = 2

} EcrioSigMgrTransportEnum;

/**
* Enumeration of constants that is used to specify the standard for checking incoming and
* outgoing message. Actually the headers checked as per 3GPP / IETF standard.
*/
typedef enum
{
	EcrioSigMgrCheckMessageNoBase = 0,		/**<Enumerated constant that represents the no base standard. This is a default setting of UAE.*/
	EcrioSigMgrCheckMessageIETFBase,		/**<Enumerated constant that represents the IETF standard. Mandatory headers will be checked as per IETF.*/
	EcrioSigMgrCheckMessage3GPPBase			/**<Enumerated constant that represents the 3GPP standard. Mandatory headers will be checked as per 3GPP.*/
} EcrioSigMgrCheckMessageStandardEnum;

typedef enum
{
	EcrioSigMgrHostTypeNoBase = -1,		/**<Enumerated constant that represents the no base standard. This is a default setting of UAE.*/
	EcrioSigMgrHostTypeIPv6,			/**<Enumerated constant that represents the IETF standard. Mandatory headers will be checked as per IETF.*/
	EcrioSigMgrHostTypeIPv4,			/**<Enumerated constant that represents the 3GPP standard. Mandatory headers will be checked as per 3GPP.*/
	EcrioSigMgrHostTypeFQDN
} EcrioSigMgrHostTypeEnum;

typedef	enum
{
	EcrioSigMgrReasonTypeDefault = 0,
	EcrioSigMgrReasonTypeRejectedByUser,
	EcrioSigMgrReasonTypeEstablishingAnotherCall,
	EcrioSigMgrReasonTypeOnTwoCalls,
	EcrioSigMgrReasonTypeNoAnswer,
	EcrioSigMgrReasonTypeOneHRPD
} EcrioSigMgrReasonPhraseEnum;

/**
*Enumeration of constants that is used to describe the message body type.
*/
typedef enum
{
	/**Enumerated constant that represents the SDP type body */
	EcrioSigMgrMessageBodySDP = 0,
	/**Enumerated constant that represents any other message body type. */
	EcrioSigMgrMessageBodyUnknown
} EcrioSigMgrMessageBodyEnum;

typedef enum
{
	EcrioSigMgrSIPRequest = 0,
	EcrioSigMgrSIPResponse
} EcrioSigMgrSIPMessageTypeEnum;

/**
*Enumeration of constants that is used to describe the header name formation.
*/
typedef enum
{
	EcrioSigMgrSipMessageModeNone = 0,
	EcrioSigMgrSipMessageModeFullName,
	EcrioSigMgrSipMessageModeShortName
} EcrioSigMgrSipMessageModeEnum;

/**
*Enumerated Constant Definition - END
*/
/**
*Constant Definition - END
*/

/**
* @def
*Structure Definition - START
*/
/**
*This structure will be passed in to info callback as pData parameter.
*@brief passed in info call as pData parameter.*/
typedef struct
{
	/** Enumeration that specifies the Event type.*/
	EcrioSigMgrMessageCmdEnum eventType;
	/** Event Identfier (Registration ID or Session Id or NULL)*/
	void *pEventIdentifier;
} EcrioSigMgrInfoStruct;

/**
*This structure will be passed in EcrioSigMgrStatusCallBack.()
*@brief passed in EcrioSigMgrStatusCallBack.
*/
typedef struct
{
	/** message command for identify type of the response*/
	EcrioSigMgrMessageCmdEnum msgCmd;
	/** Information about the response, needs to be typecasted different type of structures depending*/
	/**on the type of msgCmd*/
	void *pData;
} EcrioSigMgrMessageStruct;

/**
*This structure is used for storing the dotted IP address and the port.
*@brief Address structure.
*/
typedef struct
{
	/** IP address*/
	u_char *pIPAddr;
	/** Port number.*/
	u_int16 port;
} EcrioSigMgrIPAddrStruct;

/**
*This structure is used for storing header and/or URI parameters.
*@brief  adding parameters name and Parameters value
*/
typedef struct
{
	/** Parameter Name*/
	u_char *pParamName;
	/** Parameter Value*/
	u_char *pParamValue;
} EcrioSigMgrParamStruct;

/**
*This structure is used for specifying TEL context in case of local TEL URI.
*@brief Structure for specifying Tel context
*/
typedef struct
{
	EcrioSigMgrTelContextEnum contextType;	/**<Enumeration variable to specify context of Tel.*/
	union
	{
		u_char *pDomainName;	/**<Domain name in context.*/
		u_char *pGlobalNoDigitsInContext;/**<Global number in context.*/
	} u;
} EcrioSigMgrTelContextStruct;

/**
*This structure is used for adding different TEL parameters.
*@brief used for adding different TEL parameters
*/
typedef struct
{
	EcrioSigMgrTelParEnum telParType;/**<Enumeration that indicates the Tel parameter type.*/
	union
	{
		EcrioSigMgrParamStruct *pParameter;	/**<Tel parameters.*/
		u_char *pExtension;	/**<Tel extension.*/
		u_char *pIsdnSubaddress;	/**<ISDN sub address.*/
	} u;
} EcrioSigMgrTelParStruct;

/**
*This structure is used for specifying local telephone number details.
*@brief Structure specfying Local Telephone number details
*/
typedef struct
{
	u_char *pLocalNumberDigits;	/**<Local Tel number.*/
	u_int16 numTelPar1;	/**<Number of Tel parameters 1.*/
	EcrioSigMgrTelParStruct **ppTelPar1;/**<List of Tel parameters 1.*/
	EcrioSigMgrTelContextStruct *pContext;	/**<Tel context.*/
	u_int16 numTelPar2;	/**<Number of Tel parameters 2.*/
	EcrioSigMgrTelParStruct **ppTelPar2;/**<List of Tel parameters 2.*/
} EcrioSigMgrTelLocalNumberStruct;

/**
*This structure is used for storing global telephone number information.
*@brief Structure for holding Global Telephone number information
*/
typedef struct
{
	u_char *pGlobalNumberDigits;	/**<Global Tel number.*/
	u_int16 numTelPar;	/**<Number of Tel parameters*/
	EcrioSigMgrTelParStruct **ppTelPar;	/**<List of Tel parameters.*/
} EcrioSigMgrTelGlobalNumberStruct;

/**
*This structure is used to store Tel URI details.
*@brief used to store Tel URI details
*/
typedef struct
{
	EcrioSigMgrTelSubscriberEnum subscriberType;/**<Tel subscription type.*/
	union
	{
		EcrioSigMgrTelGlobalNumberStruct *pGlobalNumber;/**<Global Tel number.*/
		EcrioSigMgrTelLocalNumberStruct *pLocalNumber;	/**<Local Tel number.*/
	} u;
} EcrioSigMgrTelURIStruct;

/**
*This structure is used to store SIP URI details.
*@brief used to store SIP URI details
*/
typedef struct
{
	/** User ID to be used for construction of SIP URI */
	u_char *pUserId;
	/** Password to be used for construction of SIP URI [optional] */
	u_char *pPassword;
	/** domain name to be used for construction of AOR.*/
	/* User should provide either domain name or IP address*/
	u_char *pDomain;
	/** IPAddr and Port to be used for construction of CONTACT URI.*/
	EcrioSigMgrIPAddrStruct *pIPAddr;
	/** number of URI Params. SigMgr will incorporate them in <> brackets.*/
	u_int16 numURIParams;
	/** List of URI params name and Value */
	EcrioSigMgrParamStruct **ppURIParams;
	/** number of URI Headers. SigMgr will incorporate them in <> brackets.*/
	u_int16 numURIHeaders;
	/** List of URI header Name and Value */
	EcrioSigMgrParamStruct **ppURIHeaders;
} EcrioSigMgrSipURIStruct;

/**
*This structure is used for specifying a generic URI. The URI can be a SIP/TEL or custom URI.
*@brief used for specifying a generic URI
*/
typedef struct
{
	/** URI scheme to be used for construction of URI */
	EcrioSigMgrURISchemesEnum uriScheme;
	/** SIP/TEL/abs URI Information*/
	union
	{
		EcrioSigMgrSipURIStruct *pSipUri;	/**<SIP URI.*/
		EcrioSigMgrTelURIStruct *pTelUri;	/**<TEL URI.*/
		u_char *pAbsUri;						/**<Absolute URI.*/
	} u;
} EcrioSigMgrUriStruct;

/**
*This structure is used for specifying a URI in name-address format.
*@brief used for name-addr format.
*/
typedef struct
{
	/** Display Name to be used in URI header values*/
	u_char *pDisplayName;
	/** addr-spec: SIP URI/SIPS URI/abs URI/TEL URI */
	EcrioSigMgrUriStruct addrSpec;
} EcrioSigMgrNameAddrStruct;

/**
* This structure is used for specifying header values of type
* name-addr and the corresponding header parameters.
*@brief used for specifying address info.
*/
typedef struct
{
	/** URI information in Name-Address format.*/
	EcrioSigMgrNameAddrStruct nameAddr;
	/** Number of parameters.*/
	u_int16 numParams;
	/** List of Params Name and Value */
	EcrioSigMgrParamStruct **ppParams;
} EcrioSigMgrNameAddrWithParamsStruct;

/**
*This structure is used for specifying header values and its parameters.
@brief used for specifying header values and Parameters.
*/
typedef struct
{
	/** Header Value */
	u_char *pHeaderValue;
	/**Number of header value parameters.*/
	u_int16 numParams;
	/** List of header Value params name and Value */
	EcrioSigMgrParamStruct **ppParams;
} EcrioSigMgrHeaderValueStruct;

/**
*This structure is used for specifying the header values and its parameters.
*@brief used for specifying header values and Parameters.*/
typedef struct
{
	/** Header Name */
	EcrioSipHeaderTypeEnum eHdrType;
	/** Header Name, this will be used in case of Custom headers*/
	u_char *pHeaderName;
	/** number of header Values.*/
	u_int16 numHeaderValues;
	/** List of Header values*/
	EcrioSigMgrHeaderValueStruct **ppHeaderValues;
} EcrioSigMgrHeaderStruct;

/**
*This structure is used for specifying route headers.
*@brief structure for specifying route headers
*/
typedef struct
{
	/** number of routes */
	u_int16 numRoutes;
	/** Route detail info*/
	EcrioSigMgrNameAddrWithParamsStruct **ppRouteDetails;
} EcrioSigMgrRouteStruct;
/**
*This structure is used for specifying contact details.
*@brief used for specifying contact details.
*/
typedef struct
{
	/** number of Contact URIs to be registered*/
	u_int16 numContactUris;
	/** list of Contact URIs to be registered*/
	EcrioSigMgrNameAddrWithParamsStruct **ppContactDetails;
} EcrioSigMgrContactStruct;

/**
*This structure is used for via header information.
*@brief used for Via Header
*/
typedef struct
{
	/** Version of SIP. this must be SIP/2.0.*/
	u_char *pSipVersion;
	/** Enumeration that stores the transport type to be used for determining next hop.*/
	EcrioSigMgrTransportEnum transport;
	/** IPaddress and Port*/
	EcrioSigMgrIPAddrStruct pIPaddr;
	/** Branch Parameter*/
	u_char *pBranch;
	/** number of Via Params.*/
	u_int16 numParams;
	/** List of via params name and Value */
	EcrioSigMgrParamStruct **ppParams;
} EcrioSigMgrViaStruct;

/**
*This structure is used for storing mandatory header information.
*@brief used for Mandatory headers.
*/
typedef struct
{
	/** Request URI*/
	EcrioSigMgrUriStruct *pRequestUri;
	/** TO Header*/
	EcrioSigMgrNameAddrWithParamsStruct *pTo;
	/** FROM Header*/
	EcrioSigMgrNameAddrWithParamsStruct *pFrom;
	/** Call Id Values*/
	u_char *pCallId;
	/** CSeq number*/
	u_int32 CSeq;
	/** Max Forwards Value*/
	u_int32 maxForwards;
	/** Number of via headers*/
	u_int16 numVia;
	/** Via Header list*/
	EcrioSigMgrViaStruct **ppVia;
} EcrioSigMgrMandatoryHeaderStruct;

/**
*This structure is used to store the unknown message body structure information.
*@brief used to store the unknown message body structure.
*/
typedef struct
{
	/* content data buffer */
	u_char *pBuffer;	/**<Unknown message body content data buffer.*/
	/* buffer Length in bytes */
	u_int32 bufferLength;/**<Buffer length in bytes.*/
	/* content Type */
	EcrioSigMgrHeaderValueStruct contentType;/**<Content- Type of body. [to be used only for single message body and not if part of Multipart body]*/
} EcrioSigMgrUnknownMessageBodyStruct;


/**
*/
typedef struct
{
	void *pSigMgrHandle;
	u_char* pUnknownBody;
	u_char *	pDest;
	u_int32 SmsRetryCount;
} EcrioSigMgrRetryMessageStruct;


/**
* This structure is used to provide information for single message body.
*@brief Message body structure.
*/
typedef struct
{
	/**Enumeration that indicates the message body type. */
	EcrioSigMgrMessageBodyEnum messageBodyType;
	/**Data of the message body */
	void *pMessageBody;
} EcrioSigMgrMessageBodyStruct;

/**
*This structure is used for getting authorization details from the upper layer.
*@brief used for getting Authorization details.
*/

typedef struct
{
	u_char *pAuthenticationScheme;		/**< Authentication scheme to be used, this must be 'Digest' */
	u_char *pUserName;					/**< User name */
	u_char *pRealm;						/**< Realm for which authorization details are required. */
	u_char *pNonce;						/**< Nonce value, The upper layer should return the same value as given by the Server */
	u_char *pOpaque;						/**< Opaque Value */
	u_char *pURI;						/**< request Uri */
	u_char *pResponse;					/**< response for the challenge */
	EcrioSipAuthAlgorithmEnum authAlgo;	/**< Algorithm to be used, AKA or MD5 */
	u_char *pCNonce;						/**< C nonce Value */
	u_char *pQoP;						/**< Quality of protection parameter */
	u_char *pCountNonce;					/**< Nonce count */
	u_char *pAuts;						/**< AUTS */
	EcrioSipAuthStruct *pAKARes;		/**< AKA RES */
} EcrioSigMgrAuthorizationStruct;

/**
*This structure is used for specifying authentication information received from server or UA.
*@brief or specifying Authentication information.
*/
typedef struct
{
	u_char *pAuthenticationScheme;		/** Authentication scheme to be used, this must be 'Digest'*/
	u_char *pRealm;						/** Realm for which authorization details are required.*/
	u_char *pDomain;						/** Domain Name */
	u_char *pNonce;						/** Nonce Value, this should be returned in the Authorization details */
	u_char *pOpaque;						/** Opaque Value */
	BoolEnum bStale;					/** TRUE, if credentials have become stale */
	EcrioSipAuthAlgorithmEnum authAlgo;	/** Algorithm to be used to generate the response */
	u_int16 countQoP;					/** Count of Quality of parameters received from the server */
	u_char **ppListOfQoP;				/** list of Quality of parameters received from the server */
} EcrioSigMgrAuthenticationStruct;
/**
* This structure is used for specifying the SIP Timer values from the upper layer.
*@brief used for specifying the SIP Timer values.
*/
typedef struct
{
	/* All values should be non negative, 0 denotes usage of default value as per RFC */
	u_int32 T1;		/**<Round Trip Time(RTT) value.*/        /** Range - non negative - 2*/
	u_int32 T2;		/**<Maximum retransmission interval for non-invite requests and INVITE responses.*/
	u_int32 T4;		/**<Maximum duration that a message can remain in the network.*/
	u_int32 TimerA;	/**<INVITE request retransmission interval.*/
	u_int32 TimerB;	/**<INVITE transaction timeout timer interval.*/
	u_int32 TimerD;	/**<Wait time for response retransmissions.*/
	u_int32 TimerE;	/**<Non-INVITE request retransmission interval.*/
	u_int32 TimerF;	/**<Non-INVITE transaction timeout timer interval.*/
	u_int32 TimerG;	/**<INVITE response retransmit interval.*/
	u_int32 TimerH;	/**<Wait time for ACK receipt.*/
	u_int32 TimerI;	/**<Wait time for ACK retransmits.*/
	u_int32 TimerJ;	/**<Wait time for Non-INVITE request retransmits.*/
	u_int32 TimerK;	/**<Wait time for response retransmits.*/
	u_int32 TimerM;	/**<Wait time for retransmissions of the 2xx response or any additional 2xx responses from other branches of a downstream fork of the matching request*/
} EcrioSigMgrCustomTimersStruct;

typedef struct
{
	u_int16 uNumberOfTags;
	EcrioSigMgrParamStruct **ppFeatureTagHeaderValue;
} EcrioSigMgrFeatureTagStruct;

/** @struct EcrioCPMConversationsIdStruct
* This structure defines to hold CPM Conversation Identification headers.
*/
typedef struct
{
	u_char *pConversationId;					/**< Conversation-ID. */
	u_char *pContributionId;					/**< Contribution-ID. */
	u_char *pInReplyToContId;					/**< InReplyTo-Contribution-ID. */
} EcrioSigMgrConversationsIdStruct;

typedef struct
{
	/** method type*/
	EcrioSipMessageTypeEnum eMethodType;

	/** request or response*/
	EcrioSigMgrSIPMessageTypeEnum eReqRspType;

	/** Mandatory header information*/
	EcrioSigMgrMandatoryHeaderStruct *pMandatoryHdrs;

	/** response code recceived*/
	u_int32 responseCode;

	/** reason Phrase*/
	u_char *pReasonPhrase;

	/** reason Phrase*/
	u_int32 causeCode;

	/** SipVersion*/
	u_char *pSipVersion;

	/** Authorization Hdr*/
	EcrioSigMgrAuthorizationStruct *pAuthorization;

	/** Authentication Hdr*/
	EcrioSigMgrAuthenticationStruct *pAuthentication;

	/** MinExpires Header.*/
	u_int32 *pMinExpires;

	/** Authentication Header*/
	EcrioSigMgrHeaderStruct *pAuthenticationInfo;

	/** Service-Route header */
	EcrioSigMgrRouteStruct *pServiceRoute;

	/** This Variable will maintain count of Associated URIs */
	u_int16 numPAssociatedURIs;

	/**	P-Associated-URI header field transports the set of Associated
	URIs to the registered AOR. */
	EcrioSigMgrNameAddrWithParamsStruct **ppPAssociatedURI;

	/** Route header */
	EcrioSigMgrRouteStruct *pRouteSet;
	/** Record-Route header */
	EcrioSigMgrRouteStruct *pRecordRouteSet;
	/** Contact header */
	EcrioSigMgrContactStruct *pContact;

	EcrioSigMgrHeaderStruct *pAccessNWInfo;

	/** Expires */
	u_int32 *pExpires;

	/** This Variable will maintain count of Preferred Identity*/
	u_int16 numPPreferredIdentity;

	/** The P-Preferred-Identity header field is used from a user agent to a trusted proxy to carry the identity
	the user sending the SIP message wishes to be used for the P-Asserted-Header field value that the trusted
	element will insert.*/
	EcrioSigMgrNameAddrStruct **ppPPreferredIdentity;

	u_int16 numPAssertedIdentities;

	EcrioSigMgrNameAddrStruct **ppPAssertedIdentity;

	u_int16 numReferredBy;
	EcrioSigMgrNameAddrWithParamsStruct **ppReferredBy;

	/** Optional header list */
	void *pOptionalHeaderList;

	/** Message Body*/
	EcrioSigMgrMessageBodyStruct *pMessageBody;

	EcrioSigMgrHeaderStruct *pContentType;

	/** Content-Length */
	u_int32 contentLength;

	u_int32 statusCode;
	/** Enum_TRUE if request is Invalid, sigMgr will generate its response*/
	BoolEnum isRequestInValid;

	EcrioSigMgrFeatureTagStruct *pFetaureTags;
	EcrioSigMgrConversationsIdStruct* pConvId;
	/*Routing support*/
	u_int32 eModuleId;			//EcrioSigMgrCallbackRegisteringModuleEnums

	BoolEnum bPrivacy;			// enable or disable privacy header in the request.

	u_char* pSipETag;

} EcrioSigMgrSipMessageStruct;

/** \brief This function is used for informing upper layer about error, information and warnings for a running event.
 *
 *
 *
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  None.
 *
 * @param[in] infoType					- Information level.
 * @param[in] infoCode					- Error, Information or Warning code depending on the information Type.
 * @param[in] pData						- Data specific to error code (EcrioSigMgrErrorInfoStruct*) in case of error.
 * @param[in] pCallbackData			    - Data specific to upper layer.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;Notifies upper layer for informations such as error, info and warnings for a running event.
 * @return						        None.
 * @note                                None.
 */

/* Start Support for DLL Usage */
typedef void (*EcrioSigMgrInfoCallback)
(
	EcrioSigMgrInfoTypeEnum infoType,
	s_int32 infoCode,
	EcrioSigMgrInfoStruct *pData,
	void *pCallbackData
);
/* EndSupport for DLL Usage */

/** \brief This function is used for informing the upper layer about a request / response.
 *
 *
 *
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  None.
 *
 * @param[in] pSigMgrMessageStruct	    - Contains information about the message.
 * @param[in] pCallbackData		        - Data specific to upper layer.
 *
 * <b>Transaction:</b>                 <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; Notifies upper layer for incoming requests or responses.
 * @return						        None.
 * @note                                None.
 */

/* Start Support for DLL Usage */
typedef void (*EcrioSigMgrStatusCallBack)
(
	EcrioSigMgrMessageStruct *pSigMgrMessageStruct,
	void *pCallbackData
);
/* EndSupport for DLL Usage */

typedef void (*EcrioSigMgrGetPropertyCallback)
(
	EcrioSipPropertyNameEnums eName,
	EcrioSipPropertyTypeEnums eType,
	void *pData,
	void *pContext
);

// **
// *Callback function, so that upper layer may choose to override hdr values.
// *@Param IN EcrioSigMgrMessageCmdEnum messageType - message type.
// *@Param IN void* pMessage - message hdrs.
// *@Param IN void* pCallbackData - Data specific upperLayer)
// *@Return	None.

/** \brief This function allows upper layer to override header values for an out going message.
 *
 *
 *
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  None.
 *
 * @param[in] messageType				- Message type.
 * @param[in] pMessage			        - Message headers.
 * @param[in] pCallbackData		        - Data specific to upper layer.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;Notifies upper layer to overwrite outgoing message.
 * @return						        None.
 * @note                                Data members overwritten will not be validated by signaling manager.
 *                                      It is the upper layers responsibility to overwrite data members with valid data.
 */

/*
Callback Definitions - END
*/

/*
 @def
Function Definition - START
*/

// **API to create a NULL terminated SIP URI string.
// *@Param IN SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
// *@Param IN EcrioSigMgrSipURIStruct* pSipUriInfo - SIP URI structure.
// *@Param OUT char** ppSipUri - Placeholder for SIP URI string.
// *@Return	error code.

/** \brief This function is used to create a NULL terminated SIP URI string.
 *
 *
 *
 * @pre                                 EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  Allocates memory to ppSipUri.
 *
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 * @param[in] pSipUriInfo			    - SIP URI structure.
 * @param[out] ppSipUri					- Place holder for SIP URI string.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This API will create a NULL terminated SIP URI
 *                                      string using the details provided in the EcrioSigMgrSipURIStruct.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * -  ppSipUri
 * - pSipUriInfo
 */

u_int32 EcrioSigMgrFormSipURI
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrSipURIStruct *pSipUriInfo,
	u_char **ppSipUri
);



/** \brief This function is used to create a NULL terminated Tel URI string.
*
*
*
* @pre                                 EcrioSigMgrInit() must be call.
*
* <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  Allocates memory to ppTelUri.
*
* @param[in] sigMgrHandle				- Handle to signaling manager.
* @param[in] pTelUriInfo			    - Tel URI structure.
* @param[out] ppTelUri					- Place holder for Tel URI string.
*
* <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This API will create a NULL terminated Tel URI
*                                      string using the details provided in the EcrioSigMgrTelURIStruct.
* @return						        One of the pre-defined Error Codes.
* @note                                Mandatory Structure Member
* -  ppTelUri
* - pTelUriInfo
*/

u_int32 EcrioSigMgrFormTelURI
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrTelURIStruct *pTelUriInfo,
	u_char **ppTelUri
);

/* API to copy EcrioSigMgrNameAddrWithParamsStruct.
@Param IN SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
@Param IN EcrioSigMgrNameAddrWithParamsStruct* pSrcNameAddrWithParams - NameAddrWithParams structure.
@Param IN/OUT EcrioSigMgrNameAddrWithParamsStruct** ppDestNameAddrWithParams - NameAddrWithParams structure.
@Return	error code.
*/

u_int32 EcrioSigMgrCopyNameAddrWithParamStruct
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrNameAddrWithParamsStruct	*pSrcNameAddrWithParams,
	EcrioSigMgrNameAddrWithParamsStruct	**ppDestNameAddrWithParams
);

/* API to obtain the URI scheme of a given URI.
@Param IN SIGMGRHANDLE sigMgrHandle - Handle to signaling manager.
@Param IN u_char* pUri - NULL terminated URI string constant.
@Param OUT  EcrioSigMgrURISchemesEnum* pUriScheme - URI Scheme (SIP/TEL/Custom).
@Return	error code.*/

/** \brief This function is used to obtain the URI scheme of a given URI.
 *
 *
 *
 * @pre                                 EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  None.
 *
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 * @param[in] pUri					    - NULL terminated URI string constant.
 * @param[out] pUriScheme				- URI Scheme (SIP/TEL/Custom).
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to obtain the URI scheme of a given URI.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * - pUri
 * - pUriScheme
 */
u_int32 EcrioSigMgrGetUriScheme
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pUri,
	EcrioSigMgrURISchemesEnum *pUriScheme
);

/* Modification for SUE strart */

/*
API to populate URI struct from URI string.
@Param IN SIGMGRHANDLE pSigMgrHandle - Handle to signaling manager.
@Param IN u_char* pUri - Uri string.
@Param OUT EcrioSigMgrUriStruct* pUriStruct - Uri Structure.
@Return	u_int32 Error code.*/

/** \brief This function is used to populate URI structure from the URI string.
 * (NOT SUPPORTED)
 *
 *
 * @pre                                 EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  pUriStruct to be released by upper layer.
 *
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 *
 * @param[in] pUri					    - NULL terminated URI string constant.
 * @param[out] pUriStruct				- Pointer to the URI structure.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This API is used to parse the URI string.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * - pUriStruct.
 * - pUri.
 */

u_int32 EcrioSigMgrParseUri
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pUri,
	EcrioSigMgrUriStruct *pUriStruct
);

/* Modification for SUE end */

/*
API to Abort the call irrespective of the state.
@Param IN SIGMGRHANDLE* pSigMgrHandle - Handle to signaling manager.
@Param IN SIGSESSIONHANDLE* sessionHandle - Handle to session signaling manager.
@Return	u_int32 Error code.*/

/** \brief This function is used to abort the call irrespective of the state.
 *
 *
 *
 * @pre                                 EcrioSigMgrSendInvite() or EcrioSigMgrSendInviteResponse() must be call.
 *
 * <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  None.
 *
 * @param[in] sigMgrHandle				- Handle to signaling manager.
 * @param[in] sessionHandle				- Handle to session.
 *
 * <b>Transaction:</b>                 <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; Releases all data structures and deletes the dialog related information.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * - sessionHandle
 */

u_int32 EcrioSigMgrAbortSession
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallID
);

// **
// *API for copy header struct from source to destination.
// *@Param IN SIGMGRHANDLE pSigMgrHandle - Handle to signaling manager.
// *@Param IN EcrioSigMgrHdrStruct* pSrcHdrStruct - Header Structure.
// *@Param OUT EcrioSigMgrHdrStruct* pDestHdrStruct - Header Structure.
// *@Return	u_int32 Error code.

/** \brief This function is used to copy header structure from source to destination.
 *
 *
 *
 * @pre                                   EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>	          <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;Destination header structure to be released by upper layer.
 *
 * @param[in] sigMgrHandle			        - Handle to signaling manager.
 * @param[in] pSrcHdrStruct					- Source header structure.
 * @param[out] pDestHdrStruct				- Destination header structure.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to copy header structure from source to destination.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * -    pDestHdrStruct
 * -    pHdrStruct

 */
u_int32 EcrioSigMgrCopyHeader
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrHeaderStruct *pSrcHdrStruct,
	EcrioSigMgrHeaderStruct *pDestHdrStruct
);

// **
// *API for copy URI header struct from source to destination.
// *@Param IN SIGMGRHANDLE pSigMgrHandle - Handle to signaling manager.
// *@Param IN EcrioSigMgrUriStruct* pSrcUri - Uri Header Structure.
// *@Param OUT EcrioSigMgrUriStruct* pDstUri - Uri Header Structure.
// *@Return	u_int32 Error code.

/** \brief This function is used to copy URI header structure from source to destination.
 *
 *
 *
 * @pre                                   EcrioSigMgrInit() must be call.
 *
 * <b>Memory allocation:</b>	          <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;Destination Uri structure to be freed by user.
 *
 * @param[in] sigMgrHandle			    - Handle to signaling manager.
 * @param[in] pSrcUri					- Pointer to the source URI structure.
 * @param[out] pDstUri					- Pointer to the destination URI structure.
 *
 * <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to copy URI header structure from source to destination.
 * @return						        One of the pre-defined Error Codes.
 * @note                                Mandatory Structure Member
 * -    pDstUri
 * -    pSrcUri
 */

u_int32 EcrioSigMgrCopyUriHeader
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct *pDstUri
);

#if 0
/** \brief This function is used to compare session identifiers.
*
* @pre                                 EcrioSigMgrSendInvite() or EcrioSigMgrSendInviteResponse() must be call.
*
* <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  None.
*
* @param[in] sigMgrHandle				- Handle to signaling manager.
*
* @param[in] srcSessionId				- Source session ID.
* @param[in] destSessionId				- Destination session-ID.
* @param[out] pResult					- Result - TRUE if both IDs are equal else FALSE.
*
* <b>Transaction:</b>                  <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used to compare two session identifiers.
* @return						        One of the pre-defined Error Codes.
* @note                                Mandatory Structure Member
* - srcSessionId.
* - destSessionId
* - pResult
*/

u_int32 EcrioSigMgrCompareSessionID
(
	SIGMGRHANDLE sigMgrHandle,
	SIGSESSIONHANDLE srcSessionId,
	SIGSESSIONHANDLE destSessionId,
	BoolEnum *pResult
);

// **
// *API for releasing Session identifier.
// *@Param IN SIGMGRHANDLE* pSigMgrHandle - Handle to signaling manager.
// *@Param IN SIGSESSIONHANDLE* pSessionId - Session handle.
// *@Return u_int32 Error code.

/** \brief This function is used for releasing session identifier.
*
*
*
* @pre                                 EcrioSigMgrSendInvite() or EcrioSigMgrSendInviteResponse() must be call.
*
* <b>Memory allocation:</b>	        <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  None.
*
* @param[in] sigMgrHandle				   - Handle to signaling manager.
* @param[in] pSessionId				   - Pointer to the source session ID.
*
* <b>Transaction:</b>                   <br> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;This function is used for releasing session identifier object.
* @return						        One of the pre-defined Error Codes.
* @note                                Mandatory Structure Member
* - sessionId.
*/

u_int32 EcrioSigMgrReleaseSessionId
(
	SIGMGRHANDLE sigMgrHandle,
	SIGSESSIONHANDLE *pSessionId
);
#endif

u_int32 EcrioSigMgrValidateHost
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pHost,
	EcrioSigMgrHostTypeEnum	*pEnumHostType,
	BoolEnum *pBracketFound
);

u_int32 EcrioSigMgrCompareUri
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrUriStruct *pSrcUri,
	EcrioSigMgrUriStruct *pDestUri,
	BoolEnum *result
);

u_int32 EcrioSigMgrCreateOptionalHeaderList
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppOptionalHeaderList
);

u_int32 EcrioSigMgrAddOptionalHeader
(
	SIGMGRHANDLE sigMgrHandle,
	void *pOptionalHeaderList,
	EcrioSipHeaderTypeEnum eHeaderType,
	u_int16 numValues,
	u_char **ppValues,
	u_int16 numParams,
	u_char **ppParamN,
	u_char **ppParamV
);

u_int32 EcrioSigMgrGetOptionalHeader
(
	SIGMGRHANDLE sigMgrHandle,
	void *pOptionalHeaderList,
	EcrioSipHeaderTypeEnum eHdrType,
	EcrioSigMgrHeaderStruct **ppOptHeader
);

BoolEnum EcrioSigMgrCheckPrackSupport
(
	SIGMGRHANDLE hSigMgrHandle,
	void *pOptionalHeaderList,
	BoolEnum *bRequire
);

BoolEnum EcrioSigMgrCheckPEarlyMedia//P-Early-Media
(
	SIGMGRHANDLE hSigMgrHandle,
	void *pOptionalHeaderList,
	EcrioSigMgrEarlyMediaStateEnum *EarlyMediaState
);


BoolEnum EcrioSigMgrCheckTimerSupport
(
	SIGMGRHANDLE hSigMgrHandle,
	void *pOptionalHeaderList
);

u_int32	EcrioSigMgrGetDefaultPUID
(
	SIGMGRHANDLE hSigMgrHanddle,
	EcrioSigMgrURISchemesEnum ePreferredUriType,
	EcrioSigMgrNameAddrWithParamsStruct	**ppDefaultPUID
);

u_int32 EcrioSigMgrGetSubscriptionExpireInterval
(
	SIGMGRHANDLE hSigMgrHanddle
);

u_int32 EcrioSigMgrReleaseOptionalHeaderList
(
	SIGMGRHANDLE sigMgrHandle,
	void **ppOptionalHeaderList
);

u_int32 EcrioSigMgrFillUriStruct
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrUriStruct *pUri,
	u_char *pUserID,
	EcrioSigMgrURISchemesEnum eUriType
);

u_int32 EcrioSigMgrReleaseUriStruct
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrUriStruct *pUri
);

u_int32 EcrioSigMgrGetOriginator
(
	SIGMGRHANDLE sigMgrHandle,
	EcrioSigMgrNameAddrStruct *pUri,
	u_char **ppUserID
);

u_int32	EcrioSigMgrSetFeatureTagParams
(
	SIGMGRHANDLE sigMgrHandle,
	u_int16	numFeatureTagParams,
	EcrioSigMgrParamStruct **ppFeatureTagParams
);

u_int32	EcrioSigMgrGetFeatureTag
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pFeatureTagName,
	u_char **ppFeatureTagValue
);

u_int32 EcrioSigMgrDeleteAllTransaction
(
	SIGMGRHANDLE sigMgrHandle
);

u_int32 EcrioSigMgrPopulateToHeader
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pUri,
	EcrioSigMgrNameAddrWithParamsStruct *pTo
);

EcrioSigMgrURISchemesEnum EcrioSigMgrGetUriType
(
	SIGMGRHANDLE sigMgrHandle,
	u_char *pUri
);

u_int32 EcrioSigMgrSetAppData
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallId,
	void* pAppData
);

u_int32 EcrioSigMgrGetAppData
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallId,
	void** ppAppData
);

u_int32 EcrioSigMgrCheckUpdateSessionRefresh
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pCallId,
	BoolEnum* pUpdateAllow
);

u_int32 EcrioSigMgrGetNextAppData
(
	SIGMGRHANDLE sigMgrHandle,
	void** ppAppData
);

u_int32 EcrioSigMgrSetUserAgent
(
	SIGMGRHANDLE sigMgrHandle,
	u_char* pUserAgent
);

/*
*Function Definition - END
*/

#endif /* _ECRIO_SIG_MGR_COMMON_H_ */
