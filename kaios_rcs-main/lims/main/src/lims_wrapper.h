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
* @file iota_test.h
* @brief Common definitions for the iota Main Test App.
*/

#ifndef __IOTA_TEST_H__
#define __IOTA_TEST_H__

#ifdef WIN32
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#define IOTA_TEST_MAX_ALLOWED_SESSIONS	2

#ifndef WIN32 
typedef void* IOTA_TEST_SIGNAL_HANDLE;
#endif

#define IOTA_TEST_XML_LENGTH										1024

#define IOTA_TEST_XML_VERSION										(u_char*)"1.0"
#define IOTA_TEST_XML_ENCODING									(u_char*)"UTF-8"
#define IOTA_TEST_XML_OPENING_ANGLE_BRACE						(u_char*)"<"
#define IOTA_TEST_XML_CLOSING_ANGLE_BRACE						(u_char*)">"
#define IOTA_TEST_XML_QUESTIONMARK								(u_char*)"?"
#define IOTA_TEST_XML_VERSION1									(u_char*)"xml version"
#define IOTA_TEST_XML_ENCODING1									(u_char*)"encoding"
#define IOTA_TEST_XML_EQUALTO										(u_char*)"="
#define IOTA_TEST_XML_SPACE										(u_char*)" "
#define IOTA_TEST_XML_NEWLINE										(u_char*)"\n"
#define IOTA_TEST_XML_DOUBLEQUOTES								(u_char*)"\""
#define IOTA_TEST_XML_SLASH										(u_char*)"/"
#define IOTA_TEST_XML_COLON										(u_char*)":"

#define IOTA_TEST_RCSENVELOPE_STRING								(u_char*)"rcsenvelope"
#define IOTA_TEST_RCSENVELOPE_ATTRIBUTE_DEFAULTNS				(u_char*)"xmlns"
#define IOTA_TEST_RCSENVELOPE_VALUE_DEFAULTNS					(u_char*)"urn:gsma:params:xml:ns:rcs:rcs:geolocation"
#define IOTA_TEST_RCSENVELOPE_RPID_STRING						(u_char*)"rpid"
#define IOTA_TEST_RCSENVELOPE_RPID_DEFAULT						(u_char*)"urn:ietf:params:xml:ns:pidf:rpid"
#define IOTA_TEST_RCSENVELOPE_GP_STRING							(u_char*)"gp"
#define IOTA_TEST_RCSENVELOPE_GP_DEFAULT						(u_char*)"urn:ietf:params:xml:ns:pidf:geopriv10"
#define IOTA_TEST_RCSENVELOPE_GML_STRING							(u_char*)"gml"
#define IOTA_TEST_RCSENVELOPE_GML_DEFAULT						(u_char*)"http://www.opengis.net/gml"
#define IOTA_TEST_RCSENVELOPE_GS_STRING							(u_char*)"gs"
#define IOTA_TEST_RCSENVELOPE_GS_DEFAULT						(u_char*)"http://www.opengis.net/pidflo/1.0"
#define IOTA_TEST_RCSENVELOPE_ENTITY_STRING						(u_char*)"entity"

#define IOTA_TEST_RCSUPSHLOCATION_STRING							(u_char*)"rcspushlocation"
#define IOTA_TEST_RCSUPSHLOCATION_ID_STRING						(u_char*)"id"
#define IOTA_TEST_RCSUPSHLOCATION_LABEL_STRING					(u_char*)"label"
#define IOTA_TEST_RCSUPSHLOCATION_TIMEOFFSET_STRING			(u_char*)"time-offset"
#define IOTA_TEST_RCSUPSHLOCATION_UNTIL_STRING					(u_char*)"until"
#define IOTA_TEST_RCSUPSHLOCATION_GEOPRIV_STRING				(u_char*)"geopriv"
#define IOTA_TEST_RCSUPSHLOCATION_LOCATIONINFO_STRING			(u_char*)"location-info"
#define IOTA_TEST_RCSUPSHLOCATION_CIRCLE_STRING					(u_char*)"Circle"
#define IOTA_TEST_RCSUPSHLOCATION_SRSNAME_STRING				(u_char*)"srsName"
#define IOTA_TEST_RCSUPSHLOCATION_POS_STRING					(u_char*)"pos"
#define IOTA_TEST_RCSUPSHLOCATION_RADIUS_STRING				(u_char*)"radius"
#define IOTA_TEST_RCSUPSHLOCATION_UOM_STRING					(u_char*)"uom"
#define IOTA_TEST_RCSUPSHLOCATION_USAGERULES_STRING			(u_char*)"usage-rules"
#define IOTA_TEST_RCSUPSHLOCATION_RETENTIONEXPIRY_STRING		(u_char*)"retention-expiry"
#define IOTA_TEST_RCSUPSHLOCATION_TIMESTAMP_STRING				(u_char*)"timestamp"

#define IOTA_TEST_FQDN_HOST_NAME			(char *)"dfw.rcs.mavenir.com"
#define IOTA_TEST_FQDN_DNS_SERVER			(char *)"8.8.8.8"
#define MSRP_CHUNK_SIZE						1048576

#define IOTA_TEST_ACCESSTYPE									(u_char*)"IEEE-802.11"
#define IOTA_TEST_ACCESSINFO_VALUE								(u_char*)"i-wlan-node-id=000000000001"
#define IOTA_TEST_LAST_ACCESSTYPE								(u_char*)"3GPP-E-UTRAN-FDD"
#define IOTA_TEST_LAST_ACCESSINFO_VALUE							(u_char*)"utran-cell-id-3gpp=310410000b0038000"



/** @enum iotaTestMessageTypeEnum
* This enumerations defines the type of message sent, text or richcard or anything else.
*/
typedef enum
{
	iotaTestMessageTypeTEXT,					/**< The message is Text message.*/
	iotaTestMessageTypeRICHCARD						/**< The message is Richcard, in json format */
} iotaTestMessageTypeEnum;

typedef struct
{
	u_char *entity;
	u_char *pushLocationId;
	u_char *pushLocationLabel;
	int timeOffset;
	u_char *until;
	u_char *srsName;
	u_char *pos;
	u_char *uom;
	int radius;
	u_char *retentionExpiry;
	u_char *timestamp;
} iotaTestPushLocationStruct;

typedef struct
{
	LIMSHANDLE limsHandle;
	LOGHANDLE logHandle;
	MUTEXHANDLE mutexHandle;

	char *pSessionId;
	char *pGroupSessionId;
	char *pReferId;
	u_int32 eContentTypes; //bitmask with EcrioCPMContentTypeEnum

	PALINSTANCE palLimsInstance;

	char localInterface[64];

	char localAddress[64];

	char calleeNumber[32];

	char confFactoryUri[128];

	char configFile[128];

	char message[160];

	int codecListType;

	unsigned char seed[63];
	unsigned char seedHex[17];
	unsigned char localPath[64];
	unsigned char remotePath[64];

	bool bRegistered;
	bool bAirplaneModeOn;
	bool bIsLargeMode;
	unsigned char convID[64];
	unsigned char contID[64];
	unsigned char inReplyTo[64];
	EcrioCPMConversationsIdStruct convIDStruct;
	void *hQueue;
	char audioCaptureDeviceName[64];
	char audioRenderDeviceName[64];

	char fileName[64];
	char mimeType[64];
	unsigned int fileSize;
	char fileTransferId[32];
	bool bIsFileSender;

	BoolEnum bIsGroupChat;
	BoolEnum bIsClosed;
	unsigned int numofParticipants;
	char participants[4][128];

	unsigned char readBuf[1048576];
	unsigned int sentSize;
	unsigned int receivedSize;
	bool bIsEofReached;
	BoolEnum bIsCPIMWrapped;
#if WIN32
	HANDLE hIPSecHandle;
	HANDLE hThreadReady;
	DWORD ipsecthreadId;
#else
	unsigned int isRun;
	IOTA_TEST_SIGNAL_HANDLE hSignal;
#endif
	
	BoolEnum bGrAuto;
	char GrSessionId[128][128]; // max 100 session is allowd
	char GrSessionIdGroup[128][128]; // max 100 session is allowd
	int  GrSessionCount;

	BoolEnum bGrAutoAccept;
	BoolEnum bGrAutoBusy;
	BoolEnum bGrAutoUnavilabe;
	BoolEnum bGrAutoDecline;
	char *pETag;

	char paniType[64];
	char paniInfo[64];
	char planiType[64];
	char planiInfo[64];

} iotaTestStateStruct;

/** @struct iotaTextRCSStruct
* This structure defines to hold Text Message or RichCard that will be sent 
* from the client. 
*/
typedef struct
{
	u_char Message;
	unsigned long uMsgLen;			/**< The length of the data */
}iotaTextRCSStruct;

#if 0
/** @struct iotaMessageStruct
* This structure holds the data and Bot Suggestion that is sent by the client.
*/
typedef struct
{
	iotaTestMessageTypeEnum emsgType;	/**< The type of message to be sent, currently only Richcard and Text supported */
	iotaTextRCSStruct stData;			/**< RCS Data structure that is sent */
	iotaBotSuggestionStruct stBot;		/**< RCS suggested Chip List structure to be sent.This may or may not contain data  */
}iotaMessageStruct;
#endif

EMSCRIPTEN_KEEPALIVE unsigned int iota_test_init
(
void
);

EMSCRIPTEN_KEEPALIVE unsigned int iota_test_deinit
(
void
);

EMSCRIPTEN_KEEPALIVE unsigned int iota_test_register
(
void
);

EMSCRIPTEN_KEEPALIVE unsigned int iota_test_deregister
(
void
);


EMSCRIPTEN_KEEPALIVE unsigned int iota_test_SendStandAloneMessage
(
char* pText
);


/*
int iota_test_printf
(
const char *format,
...
);
*/
#define iota_test_printf(m, ...)	printf(m,##__VA_ARGS__);

#endif /* #ifndef __IOTA_TEST_H__ */
