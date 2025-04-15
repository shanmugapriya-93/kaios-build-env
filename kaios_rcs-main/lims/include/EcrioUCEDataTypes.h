/******************************************************************************

Copyright (c) 2019-2020 Ecrio, Inc. All Rights Reserved.

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

#ifndef __ECRIOUCEDATATYPES_H__
#define __ECRIOUCEDATATYPES_H__

/* Include data types header file */
#include "EcrioPAL.h"

/** \enum EcrioUCEUriTypeEnum
* Enumeration constants to identify the type of URI
*/
typedef enum
{
	ECRIO_UCE_UriType_None = 0,   /**< URI of type None */
	ECRIO_UCE_UriType_SIP,		  /**< URI of type SIP URI */
	ECRIO_UCE_UriType_TEL,		  /**< URI of type TEL URI */
	ECRIO_UCE_UriType_Unknown	  /**< any other URI scheme */
} EcrioUCEUriTypeEnum;

/** \enum EcrioUCEServiceIDEnum
* Enumeration constants to identify the different RCS Services
*/
typedef enum
{
	ECRIO_UCE_ServiceID_None 							= 0x00000000,	/**< No Service */
	ECRIO_UCE_ServiceID_StandaloneMessaging 			= 0x00000001,	/**< Standalone Messaging Service */
	ECRIO_UCE_ServiceID_Chat 							= 0x00000002,	/**< Chat Session Service */	
	ECRIO_UCE_ServiceID_VoLTE 							= 0x00000004,	/**< VoLTE Service */
	ECRIO_UCE_ServiceID_Video 							= 0x00000008,	/**< Video Service */
	ECRIO_UCE_ServiceID_QueryCapsUsingPresence 			= 0x00000010,   /**< Querying Capability Service */
	ECRIO_UCE_ServiceID_PostCall						= 0x00000020, 	/**< Post - Call */
	ECRIO_UCE_ServiceID_FileTransfer	 				= 0x00000040,	/**< File Transfer */
	ECRIO_UCE_ServiceID_FileTransferSMS 				= 0x00000080,	/**< File Transfer via SMS */	
	ECRIO_UCE_ServiceID_FileTransferHTTP 				= 0x00000100,	/**< File Transfer via HTTP Service */
	ECRIO_UCE_ServiceID_FileTransferMSRP 				= 0x00000200,	/**< File Transfer via MSRP Service */
	ECRIO_UCE_ServiceID_FileTransferThumbnail 			= 0x00000400,	/**< File Transfer Thumbnail */
	ECRIO_UCE_ServiceID_Chatbot							= 0x00000800,  	/**< Chatbot role */
	ECRIO_UCE_ServiceID_ChatbotUsingSession				= 0x00001000, 	/**< Chatbot Communication using sessions */
	ECRIO_UCE_ServiceID_ChatbotUsingStandaloneMessage	= 0x00002000, 	/**< Chatbot Communication using Standalone Messaging */
	ECRIO_UCE_ServiceID_GeolocationPush					= 0x00004000, 	/**< Geolocation PUSH */
	ECRIO_UCE_ServiceID_GeolocationPushSMS				= 0x00008000, 	/**< Geolocation PUSH via SMS */
	ECRIO_UCE_ServiceID_CallComposerEnrichedCalling		= 0x00010000, 	/**< Call composer via Enriched Calling Session */
	ECRIO_UCE_ServiceID_CallComposerMultimediaTelephony	= 0x00020000, 	/**< Call composer via Multimedia Telephony session */
	ECRIO_UCE_ServiceID_SharedMap						= 0x00040000, 	/**< Shared Map */
	ECRIO_UCE_ServiceID_SharedSketch					= 0x00080000 	/**< Shared Sketch */
} EcrioUCEServiceIDEnum;

/**
* Enumeration of constants for Capabilty types.
*/
typedef enum
{
	ECRIO_UCE_Capabilty_Type_NONE = 0,
	ECRIO_UCE_Capabilty_Type_SUBSCRIBENOTIFY,
	ECRIO_UCE_Capabilty_Type_OPTIONS

}EcrioUCECapabilityType;

/**
* Enumeration of constants of various types of VoLTE services.
*/
typedef enum
{
	EcrioUCE_VoLTE_SERVC_None = 0,				/**< VoLTE service  - None */
	EcrioUCE_VoLTE_SERVC_Full,					/**< VoLTE service  - full */
	EcrioUCE_VoLTE_SERVC_Half,					/**< VoLTE service  - half */
	EcrioUCE_VoLTE_SERVC_ReceiveOnly,			/**< VoLTE service  - receive-only */
	EcrioUCE_VoLTE_SERVC_SendOnly				/**< VoLTE service  - send-only */
} EcrioUCEVoLTEServiceDuplexEnum;

/** @struct EcrioUCEUserCapabilityListStruct
* This structure holds the URI list of users whos capabilities need to be queried.
*/
typedef struct
{
	u_int32 uNumOfUsers;								/**< Number of participants. */
	u_char **ppUri;										/**< Pointer to the URI lists. */
} EcrioUCEUserCapabilityListStruct;

/** @struct EcrioUCEUserOtionsRequestStruct
* The structure to provide information for OPTIONS reuest.
*/
typedef struct
{
	u_char *pCallId;					/**< Call ID for within Diolog OPTIONS, otherwise should be set as NULL. */
	u_char *pDest;						/**< User ID to send the OPTIONS. */
	EcrioUCEUriTypeEnum eURIType;       /**< URI type, of the user. */
	u_int32 uOptionsFeatures;			/**< OPTIONS feature, this is the lims_NotifyOptionsFeaturesEnums with bitmask. */
}EcrioUCEOptionsRequestStruct;


/** @struct EcrioUCESubscribeRequestStruct
* The structure to provide information for SUBSCRIBE reuest.
*/
typedef struct
{
	u_char *pRLSUri;			                /** < The Resource List Server URI.This is used when query the capabilities of multiple presentities.If only query for single presentity, this can be specified to NULL.> */
	EcrioUCEUserCapabilityListStruct  *pUsers;   /** <The list of users whose capabilities need to be queried.> */
}EcrioUCESubscribeRequestStruct;

/** @struct EcrioUCEQueryCapabilitiesStruct;
* This structure holds the URI list of users whos capabilities need to be queried.
*/
typedef struct
{
	EcrioUCECapabilityType		         eQueryType;
	union{
		EcrioUCESubscribeRequestStruct  *pSubscribe;
		EcrioUCEOptionsRequestStruct	*pOptions;
	}u;

} EcrioUCEQueryCapabilitiesStruct;


/**
* This structure is used for storing the details of the duplex service Capability that has to be sent for capability exchange.
* @brief Stores the information about Duplex Service Capability that has to be sent for capability exchange.
*/
typedef struct
{
	EcrioUCEVoLTEServiceDuplexEnum	    eDuplexType;		/**< Duplex type */
	BoolEnum							bSupported;			/**< A flag to indicate status of "Supported" */
} EcrioUCEVoLTEServiceDuplexStruct;

/**
* This structure is used for storing the details of the VoLTE service Capability that has to be sent for capability exchange.
* @brief Stores the information about VoLTE Service Capability that has to be sent for capability exchange.
*/
typedef struct
{
	BoolEnum								bServc;				/**< A flag to indicate status of "Service" */
	BoolEnum								bAudio;				/**< A flag to indicate status of "Audio" */
	BoolEnum								bVideo;				/**< A flag to indicate status of "Video"  */
	EcrioUCEVoLTEServiceDuplexStruct	    duplex;				/**< Structure of the duplex service capability */
} EcrioUCEVoLTEServiceStruct;


/** @struct EcrioUCEURIStruct
* The structure holds the URI.
*/
typedef struct
{
	u_char* pURI;										/**< NULL terminated URI string */
	EcrioUCEUriTypeEnum eUriType;						/**< Identifies the URI scheme is either SIP or TEL URI */
}EcrioUCEURIStruct;

/** \brief Service Information
*
* Contains the communication contact for a service, identified by its ID.
*/
typedef struct
{
	EcrioUCEServiceIDEnum eID;				/**< Service ID */
	EcrioUCEVoLTEServiceStruct volte;		/**< VoLTE service capability used if the service is VoLTE or Video */
	EcrioUCEURIStruct *pContact;			/**< Contact URI of the Presentity for this service */
	u_int16 numOfContacts;					/**< Number of contacts. */
	u_char *pTimestamp;						/**< Timestamp the service was published */
} EcrioUCEServiceDetailsStruct;

/** \brief Reason Information
*
* Structure provides the cause of the subscription for the query capabilities.
*/
typedef struct
{
	u_int32 uReasonCause;								/**< Reason cause. */
	u_char *pReasonText;								/**< Reason text. */
	u_char *pCallId;									/**< Call-Id of the request. */
} EcrioUCEReasonStruct;

/** \brief Presence Information of the User
*
* Structure used to publish the User's Service Capability Information.
*/
typedef struct
{
	EcrioUCEServiceDetailsStruct* pServices;			/**< Description of Supported Services */
	u_int16 uServiceCount;								/**< Number of supported Services */
} EcrioUCEUserCapabilityInfoStruct;

/** \brief Presence Information of a contact
*
* Structure used to represent the Contact's Service Capability Information.
* 
*/
typedef struct
{
	EcrioUCEURIStruct presentityURI;					/**< URIs of the Presentity publishing the document */
	EcrioUCEServiceDetailsStruct **ppServices;			/**< Description of Supported Services */
	u_int16 uServiceCount;								/**< Number of supported Services */
} EcrioUCEPresenceDocumentStruct;


/** \brief Records of Presence Information documents of users structure
*
* Structure used to notify the presence capability of users.
*
*/
typedef struct
{
	EcrioUCEPresenceDocumentStruct **ppRecords;		/**< Records of user presence documents */
	u_int32 uRecordCount;							/**< Number of records */
} EcrioUCERecordsOfPresenceDocumentStruct;


/** @struct EcrioUCENotifyCallBackStruct
* The structure provide information for NOTIFY request.
*/

typedef EcrioUCERecordsOfPresenceDocumentStruct EcrioUCENotifyCallBackStruct;

/** @struct EcrioUCEOptionsCallBackStruct
* The structure provides capabilities received in OPTIONS responses.
* The struture will be received in pData of CallBack function.
*/
typedef struct
{
	u_int32 uOptionsFeatures;								/**< OPTIONS feature, this is the lims_NotifyOptionsFeaturesEnums with bitmask. */
	EcrioUCEURIStruct* pPresentityURI;						/**< The P-Asserted-Identity or P-Preffered-Identity URI list */
	u_int16 uNumberOfPresentity;							/**< Number of P-Asserted-Identity or P-Preffered-Identity URI */
}EcrioUCEOptionsCallBackStruct;


/** \brief Records of Presence Information documents of users structure
*
* Structure used to notify the presence capability of users.
*
*/
typedef struct
{
	EcrioUCECapabilityType eCapabilityType;		/**< EcrioUCECapabilityType SUBSCRIBE,OPTIONS */
	void *pCapabilities;							 
} EcrioUCECapabilityDiscoveryStruct;


/** @enum EcrioUCEPropertyNameEnums
* This enumeration defines the various property names that a property obtained via the UCEGetPropertyCallback()
* function from application layer.
*/
typedef enum
{
	EcrioUCEPropertyName_NONE,						/**< No Property . */
	EcrioUCEPropertyName_OPTIONSFEATURETAG			/**< Indicates the OPTIONS feature tag . */
} EcrioUCEPropertyNameEnums;


/**
* This function will be called for obtaining properties to the upper layer.
* In particular for the UCE module this callback function will be called
* when UCE will populate various application defined features for UCE.
*
* @param[in] eName			The property name.
* @param[in] pData			A void pointer to the associated data structure
*							for obtaining properties.
* @param[in] pContext		Pointer to any optional data provided by the
*							calling layer, opaque to the UCE module. This
*							pointer would have been provided to the UCE
*							during initialization.
*/
typedef void(*EcrioUCEGetPropertyCallback)
(
	EcrioUCEPropertyNameEnums eName,
	void *pData,
	void *pContext
);


#endif /* #ifndef __ECRIOUCEDATATYPES_H__ */
