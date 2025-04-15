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

#ifndef  _ECRIO_COMMON_ENGINE_H_
#define  _ECRIO_COMMON_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Declarations - Start */

typedef void *ECRIOCOMMONSIGSETTINGHANDLE;
typedef void *ECRIOCOMMONXDMSETTINGHANDLE;

/* Declarations - End */

/* String constants  - Start */
#define ECRIO_COMMON_ENGINE_MULTIPART_RELATED				"multipart/related"			/* multipart/related content type notification request */
#define ECRIO_COMMON_ENGINE_APPLICATION_RLMI_XML			"application/rlmi+xml"		/* rlmi+xml content type notification request */
#define ECRIO_COMMON_ENGINE_APPLICATION_PIDF_XML			"application/pidf+xml"		/* pidf+xml content type notification request */
#define ECRIO_COMMON_ENGINE_APPLICATION_WATCHERINFO_XML		"application/watcherinfo+xml"	/* watcherinfo+xml content type notification request */
#define ECRIO_COMMON_ENGINE_APPLICATION_XCAPDIFF_XML		"application/xcap-diff+xml"	/* xcap-diff+xml content type notification request */
/* String constants - End */

/* Definations - Start */

/* Definations - End */

/* Enumerations - Start */

typedef enum
{
	ECRIO_ENGINE_Type_None = 0,
	ECRIO_ENGINE_Type_VoIP_Engine,
	ECRIO_ENGINE_Type_Poc_Engine,
	ECRIO_ENGINE_Type_Im_Engine,
	ECRIO_ENGINE_Type_Contact_Group_Engine,
	ECRIO_ENGINE_Type_Presence_Engine,
	ECRIO_ENGINE_Type_Poc_Xdm_Engine,
	ECRIO_ENGINE_Type_VCC_Engine
} EcrioEngineTypeEnum;	/* a common engine type enum used by all the Ecrio engines. */

typedef enum
{
	ECRIO_TK_Operation_Type_Unknown = 0,
	ECRIO_TK_Operation_Type_IsUserInAcceptList,	/* others can be added later */
	ECRIO_TK_Operation_Type_IsUserInRejectList
} EcrioTkOperationTypeEnum;

/* Enumerations - End */

/* Structure - Start */

typedef struct
{
	u_char *pServerAddr;
	u_char *pServerPath;
	u_int32 serverPort;
	u_char *pUserUri;
	u_char *pUserPasswd;
	u_char *pDomain;
} EcrioTkParamStruct;

typedef struct
{
	EcrioEngineTypeEnum	engineType;					/* engine type */
	void *pTkResponseStruct;						/* void* to assign corresponding engine rsp struct */
} EcrioTkRspStruct;

typedef struct
{
	u_char *pSigServerAddr;		// Remote Server address
	u_char *pSigLocalAddr;		// Local address
	u_int32 iSigServerPort;		// SIP Server port
	u_int32 iSigLocalPort;		// local port
	u_int32	maxForwards;		// Max-Forwards
	u_char *pSigPublicId;		// public identity: SIP URI
	u_char *pSigDisplayName;		// User Display Name
	u_char *pSigPrivateId;		// private identity
	u_char *pSigPassword;		// password
	u_char *pSigDomain;			// User domain
	u_char *pSigContactId;		// SIP URI to be used in Contact Header
	void *pImslibHandler;		// imslibrary handler
	void *udpEngine;			// udpengine
	void *udpSignalingSock;		// signaling socket
	u_char *pServiceRoute;		// service route
	BoolEnum regState;			// registration state
	void *pSigWWWAuth;			// authentication
	u_int32 *pSessionCount;		// current Session Count
	BoolEnum *pbIPAB;			// Personal Alert Barring
	BoolEnum *pbISB;			// Incomming session Barring
	u_char *pUserAgent;			// user-agent value to be used
} EcrioEngineCommonSettingsStruct;

typedef struct
{
	u_char *pXdmServerAddr;	// XDM server address
	u_char *pXdmServerPath;	// XDM server path
	u_int32 iXdmServerPort;	// XDM server port
	u_char *pXdmUserUri;		// XDM User URI
	u_char *pXdmUserPasswd;	// XDM user password
	u_char *pXdmDomain;		// XDM Domain
	void *pXdmWWWAuth;		// authentication
} EcrioEngineCommonXdmSettingsStruct;

typedef struct
{
	void *pData;// input data ,this may be structure or u_char* etc.
	EcrioTkOperationTypeEnum operation;
} EcrioTkCommandStruct;

/* Structure - End */

/* API Signatures Declaration - Start */

/* callback function to provide the MMI with descriptions of ongoing operations.
    The MMI can choose to ignore updates sent via this API and return directly */
extern void EcrioEngineDisplayStatus
(
	EcrioEngineTypeEnum engineType,
	void *pTkStructPtr,					/* this pointer needs to be typecasted to correspondign engine structure pointer */
	u_int32 *pStatusEnums,				/* this pointer needs to be typecasted to respective
										engine's status enum's. e.g if engine type is Poc_engine
										then the enum would be of type EcrioPOCTkStatusEnum. Its the responsibility of the MMI to free this pointer. */
	void *pData
);

/* callback function to notify the MMI of an asynchronous error in the engine,
    library, stack or driver layers */
extern void EcrioEngineError
(
	EcrioEngineTypeEnum engineType,
	void *pTkStructPtr,							/* this pointer needs to be typecasted to correspondign engine structure pointer */
	u_int32 errorCode,							/* this is the corresponding engine's error code */
	void *pData
);

/* callback function that the MMI needs to implement to receive updates about when
    a signalling, media or glms event/response arrives */
extern u_int32 EcrioEngineUpdateMMIStatus
(
	EcrioTkRspStruct *pTkRspStruct
);

/* callback function from POC engine to other engine to get the data*/
extern u_int32 EcrioEngineAppGetInfo
(
	EcrioEngineTypeEnum tkType,
	EcrioTkCommandStruct *pCommandStruct,
	void **ppOutput
);

/* callback function to obtain a pointer to the engine structure. This function is necessary
    because the device driver interfaces might be invoked in a different context */
/* depending on the engine type the MMI would need to return the corresponding pointer. */
extern void *EcrioEngineAppGetEngineStruct
(
	EcrioEngineTypeEnum engineType
);

/* callback function to obtain a common signaling struct pointer.This function is neccesary to get
   common signaling settings from main engine to other engines */
extern void *EcrioEngineAppGetCommonSignalingSettings
(
	void
);
/* callback function to obtain a common xdm parameter structure pointer from MMI.This function is neccessary to get
   common xdm settings from application to other engines */
extern void *EcrioEngineAppGetCommonXdmSettings
(
	void
);
/* callback function to set the common xdm settings.Engine whoever has the xdm common setting will set it to MMI.MMI will
   provide to other engines. Note: Caller need to allocate memory and assign the approriate values
   for input parameter ppCommonXdmSettings */
extern void EcrioEngineAppSetCommonXdmSettings
(
	void **ppCommonXdmSettings
);
/* callback function on Socket Receive Data */
extern void EcrioEngineSignalingRecvCallback
(
	void *pLibRspStruct
);

/* callback function on Socket Send Data */
extern void EcrioEngineSignalingSendCallback
(
	void *pCallbackFnData,
	u_int32 contextValue
);

/* API Signatures Declaration - End */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ECRIO_COMMON_ENGINE_H_ */
