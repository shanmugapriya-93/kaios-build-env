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
 * @file iota_test_config.cpp
 * @brief Configuration access for the iota Main Test App.
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include "ini.h"

#include "EcrioPAL.h"
#include "lims.h"

#include "iota_test.h"

#ifdef WIN32
// There is a problem since we are linking some release libraries (such as codecs) in
// that the release heap is also loaded. For some reason strdup falls for the release
// library in debug mode. So we will remap to _strdup which seems to fix the problem.
// https://stackoverflow.com/questions/8740500/heap-corruption-with-strdup
// @todo Maybe there is a better way to avoid the extra library from being loaded?
#define strdup    _strdup
#endif

extern iotaTestStateStruct iotaState;

typedef struct
{
	iotaMessageStruct *pMsg;
	char *pRchStr;
	char *pSclStr;
} iotaTestMessageStruct;

// inih (configuration file) callback for configuration settings.
int iota_test_config_iniConfigHandler
(
	void *user,
	const char *section,
	const char *name,
	const char *value
)
{
	lims_ConfigStruct *pConfig = (lims_ConfigStruct *)user;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("general", "home_domain"))
	{
		pConfig->pHomeDomain = strdup(value);
	}
	else if (MATCH("general", "password"))
	{
		pConfig->pPassword = strdup(value);
	}
	else if (MATCH("general", "private_identity"))
	{
		pConfig->pPrivateIdentity = strdup(value);
	}
	else if (MATCH("general", "public_identity"))
	{
		pConfig->pPublicIdentity = strdup(value);
	}
	else if (MATCH("general", "user_agent"))
	{
		pConfig->pUserAgent = strdup(value);
	}
	else if (MATCH("general", "reg_expire_interval"))
	{
		pConfig->uRegExpireInterval = strtoul(value, NULL, 10);
	}
	else if (MATCH("general", "sub_expire_interval"))
	{
		pConfig->uSubExpireInterval = strtoul(value, NULL, 10);
	}
	else if (MATCH("general", "pub_expire_interval"))
	{
		pConfig->uPublishRefreshInterval = strtoul(value, NULL, 10);
	}
	else if (MATCH("general", "subscribe_to_reg"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bSubscribeRegEvent = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bSubscribeRegEvent = Enum_FALSE;
		}
	}
	else if (MATCH("general", "publish_capability_info"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bCapabilityInfo = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bCapabilityInfo = Enum_FALSE;
		}
	}
	else if (MATCH("general", "unsubscribe_to_reg"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bUnSubscribeRegEvent = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bUnSubscribeRegEvent = Enum_FALSE;
		}
	}
	else if (MATCH("general", "support_chatbot"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bisChatbot = Enum_TRUE;
		}
		else if(strcmp(value, "false") == 0)
		{
			pConfig->bisChatbot = Enum_FALSE;
		}
	}
	else if (MATCH("general", "send_ringing_response"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bSendRingingResponse = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bSendRingingResponse = Enum_FALSE;
		}
	}
	else if (MATCH("general", "auth_algorithm"))
	{
		if (strcmp(value, "md5") == 0)
		{
			pConfig->eAlgorithm = EcrioSipAuthAlgorithmMD5;
		}
		else if (strcmp(value, "aka_v1") == 0)
		{
			pConfig->eAlgorithm = EcrioSipAuthAlgorithmAKAv1;
		}
		else if (strcmp(value, "aka_v2") == 0)
		{
			pConfig->eAlgorithm = EcrioSipAuthAlgorithmAKAv2;
		}
		else
		{
			// @temp Default to MD5...
			pConfig->eAlgorithm = EcrioSipAuthAlgorithmMD5;
		}
	}
	else if (MATCH("general", "device_id"))
	{
		pConfig->pDeviceId = strdup(value);
	}
	else if (MATCH("general", "display_name"))
	{
		pConfig->pDisplayName = strdup(value);
	}
	else if (MATCH("general", "oom"))
	{
		if (strcmp(value, "vzw") == 0)
		{
			//pConfig->pOOMObject = vzw_oom_GetObject();
		}
		else if (strcmp(value, "att") == 0)
		{
			//pConfig->pOOMObject = att_oom_GetObject();
		}
		else if (strcmp(value, "undefined") == 0)
		{
			//pConfig->pOOMObject = undefined_oom_GetObject();
		}
		else
		{
			// @temp Default to default oom...
			pConfig->pOOMObject = default_oom_GetObject();
		}
	}
	else if (MATCH("general", "mtu"))
	{
		pConfig->uMtuSize = strtoul(value, NULL, 10);
	}
	else if (MATCH("general", "keep_alive"))
	{
		pConfig->uNoActivityTimerInterval = strtoul(value, NULL, 10);
	}
	else if (MATCH("general", "enable_udp"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bEnableUdp = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bEnableUdp = Enum_FALSE;
		}
	}
	else if (MATCH("general", "enable_tcp"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bEnableTcp = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bEnableTcp = Enum_FALSE;
		}
	}
	else if (MATCH("general", "enable_pai"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bEnablePAI = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bEnablePAI = Enum_FALSE;
		}
	}
	else if (MATCH("general", "pani"))
	{
		pConfig->pPANI = strdup(value);
	}


	else
	{
		return 0;	/* unknown section/name, error */
	}

	return 1;
}

// inih (configuration file) callback for network settings.
int iota_test_config_iniNetworkHandler
(
	void *user,
	const char *section,
	const char *name,
	const char *value
)
{
	lims_NetworkConnectionStruct *pNetwork = (lims_NetworkConnectionStruct *)user;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("network", "remote_ip"))
	{
		// We read in all "remote_ip" values and add to dynamic array. We limit
		// the number of IPs to 3 and ignore any subsequent ones.
		if (pNetwork->uNoOfRemoteIps == 0)
		{
			// Allocate for the first time.
			pNetwork->ppRemoteIPs = (char **)calloc(3, sizeof(char *));
		}

		if (pNetwork->uNoOfRemoteIps < 3)
		{
			pNetwork->ppRemoteIPs[pNetwork->uNoOfRemoteIps] = strdup(value);
			pNetwork->uNoOfRemoteIps++;
		}
		else
		{
			iota_test_printf("Ignoring Remote IP %s\n", value);
		}
	}
	else if (MATCH("network", "remote_port"))
	{
		pNetwork->uRemotePort = strtoul(value, NULL, 10);
	}
	else if (MATCH("network", "remote_tls_port"))
	{
		pNetwork->uRemoteTLSPort = strtoul(value, NULL, 10);
	}
	else if (MATCH("network", "local_port"))
	{
		pNetwork->uLocalPort = strtoul(value, NULL, 10);
	}
	else
	{
		return 0;	/* unknown section/name, error */
	}

	return 1;
}

int iota_test_config_iniMsrpRelayHandler
(
	void *user,
	const char *section,
	const char *name,
	const char *value
)
{
	lims_ConfigStruct *pConfig = (lims_ConfigStruct *)user;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("msrp-relay", "enable"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bIsRelayEnabled = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bIsRelayEnabled = Enum_FALSE;
		}
	}
	else if (MATCH("msrp-relay", "relay_ip"))
	{
		pConfig->pRelayServerIP = strdup(value);
	}
	else if (MATCH("msrp-relay", "relay_port"))
	{
		pConfig->uRelayServerPort = strtoul(value, NULL, 10);
	}
	else
	{
		return 0;	/* unknown section/name, error */
	}

	return 1;
}

int iota_test_config_iniTLSHandler
(
void *user,
const char *section,
const char *name,
const char *value
)
{
	lims_ConfigStruct *pConfig = (lims_ConfigStruct *)user;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("tls", "enable"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bTLSEnabled = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bTLSEnabled = Enum_FALSE;
		}
	}
	else if (MATCH("tls", "peer_verification"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bTLSPeerVerification = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bTLSPeerVerification = Enum_FALSE;
		}
	}
	else if (MATCH("tls", "tls_certificate"))
	{
		pConfig->pTLSCertificate = strdup(value);
	}
	else
	{
		return 0;	/* unknown section/name, error */
	}

	return 1;
}

int iota_test_config_iniPrecondition
(
	void *user,
	const char *section,
	const char *name,
	const char *value
)
{
	lims_ConfigStruct *pConfig = (lims_ConfigStruct *)user;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("precondition", "supported"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bSupportedPrecondition = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bSupportedPrecondition = Enum_FALSE;
		}
	}
	else if (MATCH("precondition", "required"))
	{
		if (strcmp(value, "true") == 0)
		{
			pConfig->bRequiredPrecondition = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			pConfig->bRequiredPrecondition = Enum_FALSE;
		}
	}
	else
	{
		return 0;	/* unknown section/name, error */
	}

	return 1;
}

// inih (configuration file) callback to get testing Rich card and SCL data from config file
int iota_test_config_iniTestDataHandler
(
	void *user,
	const char *section,
	const char *name,
	const char *value
)
{
	iotaTestMessageStruct *pStruct = (iotaTestMessageStruct *)user;
	iotaMessageStruct *pTestData = pStruct->pMsg;
	char *pStr = NULL;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (pTestData->emsgType == iotaTestMessageTypeRICHCARD)
	{
		if (MATCH("test", "rch"))
		{
			pStr = strdup(value);
			if (pStruct->pRchStr == NULL)
			{
				pStruct->pRchStr = (char*)malloc(strlen(pStr) + 1);
				memset(pStruct->pRchStr, 0, strlen(pStr) + 1);
				strcpy(pStruct->pRchStr, pStr);
			}
			else
			{
				pStruct->pRchStr = (char*)realloc(pStruct->pRchStr, strlen(pStruct->pRchStr) + strlen(pStr) + 1);
				strncat(pStruct->pRchStr, pStr, strlen(pStr));
			}
			free(pStr);
		}
	}
	if (MATCH("test", "scl"))
	{
		pStr = strdup(value);
		if (pStruct->pSclStr == NULL)
		{
			pStruct->pSclStr = (char*)malloc(strlen(pStr) + 1);
			memset(pStruct->pSclStr, 0, strlen(pStr) + 1);
			strcpy(pStruct->pSclStr, pStr);
		}
		else
		{
			pStruct->pSclStr = (char*)realloc(pStruct->pSclStr, strlen(pStruct->pSclStr) + strlen(pStr) + 1);
			strncat(pStruct->pSclStr, pStr, strlen(pStr));
		}
		free(pStr);
	}
	else
	{
		return 0;	/* unknown section/name, error */
	}

	return 1;
}

int iota_test_config_iniDefaultsHandler
(
	void *user,
	const char *section,
	const char *name,
	const char *value
)
{
	iotaTestStateStruct *pStateStruct = (iotaTestStateStruct *)user;
	char *pStr = NULL;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("defaults", "destination_uri"))
	{
		pStr = strdup(value);
		strcpy(pStateStruct->calleeNumber, pStr);
		free(pStr);
	}
	else if (MATCH("defaults", "conf_fact_uri"))
	{
		pStr = strdup(value);
		strcpy(pStateStruct->confFactoryUri, pStr);
		free(pStr);
	}
	else
	{
		return 0;	/* unknown section/name, error */
	}
	return 1;
}


int iota_test_config_getConfig
(
	char *configFile,
	lims_ConfigStruct *config
)
{
	if (ini_parse(configFile, iota_test_config_iniConfigHandler, config) < 0)
	{
		// ScreenPrintf("Can't load %s for configuration\n", g_ConfigFile);
		return 1;
	}

	return 0;
}

int iota_test_config_getNetwork
(
	char *configFile,
	lims_NetworkConnectionStruct *network
)
{
	if (ini_parse(configFile, iota_test_config_iniNetworkHandler, network) < 0)
	{
		// ScreenPrintf("Can't load %s for network\n", g_ConfigFile);
		return 1;
	}

	return 0;
}

int iota_test_config_getMsrpRelay
(
	char *configFile,
	lims_ConfigStruct *config
)
{
	if (ini_parse(configFile, iota_test_config_iniMsrpRelayHandler, config) < 0)
	{
		return 1;
	}

	return 0;
}

int iota_test_config_getTLSParameters
(
char *configFile,
lims_ConfigStruct *config
)
{
	if (ini_parse(configFile, iota_test_config_iniTLSHandler, config) < 0)
	{
		return 1;
	}

	return 0;
}

int iota_test_config_getPrecondition
(
	char *configFile,
	lims_ConfigStruct *config
)
{
	if (ini_parse(configFile, iota_test_config_iniPrecondition, config) < 0)
	{
		return 1;
	}

	return 0;
}

int iota_test_config_getDefaults
(
	char *configFile,
	iotaTestStateStruct *testStateStruct
)
{
	if (ini_parse(configFile, iota_test_config_iniDefaultsHandler, testStateStruct) < 0)
	{
		// ScreenPrintf("Can't load %s for network\n", g_ConfigFile);
		return 1;
	}

	return 0;
}

/*function to get test data from configration file*/
int iota_test_config_getTestData
(
char *configFile,
iotaMessageStruct *ptestData
)
{
	iotaTestMessageStruct stTestData;

	stTestData.pMsg = ptestData;
	stTestData.pRchStr = NULL;
	stTestData.pSclStr = NULL;

	if (ini_parse(configFile, iota_test_config_iniTestDataHandler, &stTestData) < 0)
	{
		// ScreenPrintf("Can't load %s for media\n", g_ConfigFile);
		return 1;
	}

	if (stTestData.pRchStr != NULL)
	{
		ptestData->stData.Message.assign(stTestData.pRchStr);
		free(stTestData.pRchStr);
	}

	if (stTestData.pSclStr != NULL)
	{
		ptestData->stBot.Json.assign(stTestData.pSclStr);
		free(stTestData.pSclStr);
	}

	return 0;
}

int iota_test_config_iniRcsServiceHandler
(
void *user,
const char *section,
const char *name,
const char *value
)
{
	lims_ConfigStruct *pConfig = (lims_ConfigStruct *)user;

#define MATCH(s, n)    strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("rcs-service", "provide_geoloc_push"))
	{
		if (strcmp(value, "true") == 0)
		{
			// pConfig->bPushLocation = Enum_TRUE;
		}
		else if (strcmp(value, "false") == 0)
		{
			// pConfig->bPushLocation = Enum_FALSE;
		}
	}
	else
	{
		return 0;	/* unknown section/name, error */
	}

	return 1;
}

int iota_test_config_getRcsService
(
char *configFile,
lims_ConfigStruct *config
)
{
	if (ini_parse(configFile, iota_test_config_iniRcsServiceHandler, config) < 0)
	{
		return 1;
	}

	return 0;
}
