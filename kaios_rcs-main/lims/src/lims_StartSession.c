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
* @file lims_StartSession
* @brief Implementation of lims module start CPM session functionality.
*/

#include "lims.h"
#include "lims_internal.h"

/**
* This API can be called to start the CPM/File Transfer over MSRP session. The pTarget in
* lims_StartSessionStruct will point to a target User to start a session. Default URI
* type lims assumes internally TEL URI. For Filet Transfer over MSRP session the  contentTypes
* is set to EcrioCPMContentTypeEnum_FileTransfer and after establsihing session, content of the
* file is chunked and sent to other party via MSRP internally.
*
* @param[in] handle			The lims instance handle.
* @param[in] pStart			Pointer to lims_StartSessionStruct.
* @param[in,out] ppSessionId	The session handle.
*
* @return Returns LIMS_NO_ERROR if successful, otherwise an error specific to the
* lims implementation.
* If returned LIMS_SOCKET_ERROR, application need to re-initalize lims by invoking
* lims_Deinit(), lims_Init(), lims_NetworkStateChange() and lims_Register().
*/
u_int32 lims_StartSession
(
	LIMSHANDLE handle,
	lims_StartSessionStruct *pStart,
	char **ppSessionId
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;

#if defined(ENABLE_RCS)
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMStartSessionStruct startSessionStruct = { 0 };
	u_char sipUri[LIMS_SIP_URI_LENGTH] = { 0 };
#endif

	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return LIMS_INVALID_PARAMETER1;
	}

	m = (lims_moduleStruct *)handle;

	pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
#if 0
	if (m->sessionState != lims_Session_State_IDLE)
	{
		uLimsError = LIMS_INVALID_OPERATION;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Already in session!",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}
#endif
	if (pStart == NULL || ppSessionId == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pStart or pSessionId is NULL",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}

	if (pStart->pFile == NULL)
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pStart->pFile NULL",
				 __FUNCTION__, __LINE__);
	}

	if ((pStart->pFile != NULL) && (pStart->pFile->pAppData == NULL))
	{
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pStart->pFile->pAppData NULL",
				 __FUNCTION__, __LINE__);
	}

	if (m->moduleState == lims_Module_State_REGISTERED)
	{
		/* Check network status. Actually this check is not required. Application can check this before it invoke the startSession API. */
		if (m->pConnection->uStatus != lims_Network_Status_Success)
		{
			uLimsError = LIMS_INVALID_OPERATION;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Network status doesn't up.",
				__FUNCTION__, __LINE__);
			goto ERR_None;
		}

		if (pStart->pTarget == NULL)
		{
			uLimsError = LIMS_INVALID_PARAMETER2;
			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pTarget is NULL",
				__FUNCTION__, __LINE__);
			goto ERR_None;
		}

		if (pStart->contentTypes == EcrioCPMContentTypeEnum_None)
		{
			if (pStart->bIsChatbot == Enum_TRUE)
				startSessionStruct.contentTypes = EcrioCPMContentTypeEnum_Text | EcrioCPMContentTypeEnum_IMDN | EcrioCPMContentTypeEnum_Composing | EcrioCPMContentTypeEnum_RichCard | EcrioCPMContentTypeEnum_SuggestedChipList | EcrioCPMContentTypeEnum_SuggestionResponse | EcrioCPMContentTypeEnum_SharedClientData;
			else 
				startSessionStruct.contentTypes = EcrioCPMContentTypeEnum_Text | EcrioCPMContentTypeEnum_IMDN | EcrioCPMContentTypeEnum_Composing;
		}
		else
			startSessionStruct.contentTypes = pStart->contentTypes;
		
		startSessionStruct.connStruct.isIPv6 = (m->pConnection->eIPType == lims_Network_IP_Type_V6) ? Enum_TRUE : Enum_FALSE;
		startSessionStruct.connStruct.pLocalIp = (u_char *)m->pConnection->pLocalIp;
		startSessionStruct.connStruct.uLocalPort = pStart->uMediaPort;

		if (pal_StringNCompare(pStart->pTarget, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")) == 0)
		{
			if (pal_StringFindSubString(pStart->pTarget, (u_char*)"@"))
			{
				u_int16 count = 4;
				u_int16 len = (u_int16)pal_StringLength(pStart->pTarget);
				if (NULL == pal_StringNCopy(sipUri, LIMS_SIP_URI_LENGTH, (u_char*)"sip:", pal_StringLength((u_char*)"sip:")))//Assuming the max string length will not be greater than 256
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				while (count < len)
				{
					if (!pal_StringCompare(pStart->pTarget + count, (u_char*)"@"))
					{
						if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)"@", pal_StringLength((u_char*)"@")))
						{
							uLimsError = LIMS_NO_MEMORY;
							LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
								__FUNCTION__, __LINE__);
							goto ERR_None;
						}
						if (count + 1 >= len)
						{
							if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)m->pConfig->pHomeDomain, pal_StringLength((u_char*)m->pConfig->pHomeDomain)))
							{
								uLimsError = LIMS_NO_MEMORY;
								LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
									__FUNCTION__, __LINE__);
								goto ERR_None;
							}
							break;
						}
					}
					if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), pStart->pTarget + count, 1))
					{
						uLimsError = LIMS_NO_MEMORY;
						LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
							__FUNCTION__, __LINE__);
						goto ERR_None;
					}
					count++;
				}
				startSessionStruct.pDest = sipUri;
			}
			else
			{
				if (NULL == pal_StringNCopy(sipUri, LIMS_SIP_URI_LENGTH, pStart->pTarget, pal_StringLength(pStart->pTarget)))
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)"@", pal_StringLength((u_char*)"@")))
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				if (NULL == pal_StringNConcatenate(sipUri, LIMS_SIP_URI_LENGTH - pal_StringLength((u_char*)sipUri), (u_char*)m->pConfig->pHomeDomain, pal_StringLength((u_char*)m->pConfig->pHomeDomain)))
				{
					uLimsError = LIMS_NO_MEMORY;
					LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t Memory copy error.",
						__FUNCTION__, __LINE__);
					goto ERR_None;
				}
				startSessionStruct.pDest = sipUri;
			}
		}
		else
		{
			startSessionStruct.pDest = pStart->pTarget;
		}

		startSessionStruct.bIsLargeMode = pStart->bIsLargeMode;
		startSessionStruct.pConvId = pStart->pConvId;
		startSessionStruct.bPrivacy = pStart->bPrivacy;
		startSessionStruct.bIsGroupChat = Enum_FALSE;
		startSessionStruct.bIsChatbot = pStart->bIsChatbot;
		startSessionStruct.eBotPrivacy = pStart->eBotPrivacy;

		if (pStart->pFile != NULL)
		{
			startSessionStruct.pFile = pStart->pFile;
			LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\t File path is %s",
					 __FUNCTION__, __LINE__, startSessionStruct.pFile->pFilePath);
		}
		else
		{
			startSessionStruct.pFile = NULL;
		}

		uCPMError = EcrioCPMStartSession(m->pCpmHandle, &startSessionStruct, ppSessionId);
		if (uCPMError != ECRIO_CPM_NO_ERROR)
		{
			if (uCPMError == ECRIO_CPM_SOCKET_ERROR)
				uLimsError = LIMS_SOCKET_ERROR;
			else
				uLimsError = LIMS_CPM_STARTSESSION_ERROR;

			LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMStartSession() failed with error code uCPMError:%d",
				__FUNCTION__, __LINE__, uCPMError);
			goto ERR_None;
		}

#if 0
		m->sessionState = lims_Session_State_INVITING;
#endif
	}
	else
	{
		uLimsError = LIMS_INVALID_OPERATION;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t lims is not in REGISTERED state",
			__FUNCTION__, __LINE__);
		goto ERR_None;
	}
#else
	pStart = pStart;
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  CPM feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_CPM_FEATURE_NOT_SUPPORTED;
	goto ERR_None;
#endif

ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
		__FUNCTION__, __LINE__, uLimsError);

	pal_MutexGlobalUnlock(m->pal);

	return uLimsError;
}

/**
* This function is called to get the application data stored in the session.
*
* @param[in] handle				The lims instance handle.
* @param[in] pSessionId			The ID of the session
* @return void*					Pointer stored in the session structure
*/
void * lims_GetFileAppData
(
		LIMSHANDLE handle,
		char *pSessionId
)
{
	u_int32 uLimsError = LIMS_NO_ERROR;
	void* pAppData = NULL;

#if defined(ENABLE_RCS)
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
#endif

	lims_moduleStruct *m = NULL;

	if (handle == NULL)
	{
		return NULL;
	}

	m = (lims_moduleStruct *)handle;

	//pal_MutexGlobalLock(m->pal);

	LIMSLOGI(m->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

#if defined(ENABLE_RCS)
	if (pSessionId == NULL)
	{
		uLimsError = LIMS_INVALID_PARAMETER2;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t pStart or pSessionId is NULL",
				 __FUNCTION__, __LINE__);
		goto ERR_None;
	}

	if (m->moduleState == lims_Module_State_REGISTERED)
	{
		LIMSLOGD(m->logHandle, KLogTypeGeneral, "%s:%u\tSessionID Passed %s",
				 __FUNCTION__, __LINE__, pSessionId);
        uCPMError = EcrioCPMGetFileAppData(m->pCpmHandle, pSessionId, &pAppData);
        if (uCPMError != ECRIO_CPM_NO_ERROR)
        {
            uLimsError = LIMS_CPM_STARTSESSION_ERROR;

            LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t EcrioCPMGetFileAppData() failed with error code uCPMError:%d",
                     __FUNCTION__, __LINE__, uCPMError);
            goto ERR_None;
        }
	}
	else
	{
		uLimsError = LIMS_INVALID_OPERATION;
		LIMSLOGE(m->logHandle, KLogTypeGeneral, "%s:%u\t lims is not in REGISTERED state",
				 __FUNCTION__, __LINE__);
		goto ERR_None;
	}
#else
	LIMSLOGW(m->logHandle, KLogTypeGeneral, "%s:%u-  CPM feature disabled from this build", __FUNCTION__, __LINE__);
	uLimsError = LIMS_CPM_FEATURE_NOT_SUPPORTED;
	goto ERR_None;
#endif

	ERR_None:
	LIMSLOGI(m->logHandle, KLogTypeFuncExit, "%s:%u\t%u",
			 __FUNCTION__, __LINE__, uLimsError);

	//pal_MutexGlobalUnlock(m->pal);

	return pAppData;
}