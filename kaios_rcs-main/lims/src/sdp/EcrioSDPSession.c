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
 * @file EcrioSDPSession.c
 * @brief Implementation of the Ecrio SDP Interface session related
 * functions.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

/** Utility function to copy the audio payload structure. */
void ec_SDP_CopyAudioPayload
(
	EcrioSDPAudioPayloadStruct *dest,
	EcrioSDPAudioPayloadStruct *src
)
{
	/** @note No parameter checking or logging for this function. */

	dest->uClockrate = src->uClockrate;
	dest->uChannels = src->uChannels;
	dest->bFlag = src->bFlag;
	dest->uOpaque = src->uOpaque;
	dest->uSettings = src->uSettings;
}

/** Utility function to copy the bandwidth structure. */
void ec_SDP_CopyBandwidth
(
	EcrioSDPBandwidthStruct *dest,
	EcrioSDPBandwidthStruct *src
)
{
	/** @note No parameter checking or logging for this function. */

	dest->uModifier = src->uModifier;
	dest->uAS = src->uAS;
	dest->uRS = src->uRS;
	dest->uRR = src->uRR;
}

void ec_SDP_CopyPrecondition
(
	EcrioSDPPreconditionStruct *dest,
	EcrioSDPPreconditionStruct *src
)
{
	dest->type = src->type;
	dest->e2e.curr.strength = src->e2e.curr.strength;
	dest->e2e.curr.direction = src->e2e.curr.direction;
	dest->e2e.des.strength = src->e2e.des.strength;
	dest->e2e.des.direction = src->e2e.des.direction;
	dest->e2e.conf.strength = src->e2e.conf.strength;
	dest->e2e.conf.direction = src->e2e.conf.direction;
	dest->local.curr.strength = src->local.curr.strength;
	dest->local.curr.direction = src->local.curr.direction;
	dest->local.des.strength = src->local.des.strength;
	dest->local.des.direction = src->local.des.direction;
	dest->local.conf.strength = src->local.conf.strength;
	dest->local.conf.direction = src->local.conf.direction;
	dest->remote.curr.strength = src->remote.curr.strength;
	dest->remote.curr.direction = src->remote.curr.direction;
	dest->remote.des.strength = src->remote.des.strength;
	dest->remote.des.direction = src->remote.des.direction;
	dest->remote.conf.strength = src->remote.conf.strength;
	dest->remote.conf.direction = src->remote.conf.direction;
}

u_int32 ec_SDP_CheckSDPEvent
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPSessionStruct *pRemoteSDP = NULL;
	EcrioSDPSessionStruct *pPrevSDP = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pRemoteSDP = &S->remoteSession;
	pPrevSDP = &S->work;

	/** Check the identical SDP */
	if (pRemoteSDP->origin.uVersion == pPrevSDP->origin.uVersion)
	{
		SDPLOGW(s->logHandle, KLogTypeGeneral, "%s:%u\tDetect the identical SDP.", __FUNCTION__, __LINE__);
		pInfo->eEvent = ECRIO_SDP_EVENT_IDENTICAL;
		goto END;
	}

	/** Check whether the remote IP address has been changed */
	if ((pRemoteSDP->origin.eAddressType != pPrevSDP->origin.eAddressType) ||
		(pal_StringCompare(pRemoteSDP->origin.pAddress, pPrevSDP->origin.pAddress) != 0))
	{
		SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tRemote IP address in o= line has been changed.", __FUNCTION__, __LINE__);
		pInfo->eEvent = ECRIO_SDP_EVENT_IP_ADDRESS_CHANGED;
	}

	if (pRemoteSDP->conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
		pRemoteSDP->conn.pAddress != NULL)
	{
		if ((pRemoteSDP->conn.eAddressType != pPrevSDP->conn.eAddressType) ||
			(pal_StringCompare(pRemoteSDP->conn.pAddress, pPrevSDP->conn.pAddress) != 0))
		{
			SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tRemote IP address in c= line has been changed.", __FUNCTION__, __LINE__);
			pInfo->eEvent = ECRIO_SDP_EVENT_IP_ADDRESS_CHANGED;
		}
	}
	else
	{
		for (i = 0; i < pRemoteSDP->uNumOfMedia; ++i)
		{
			if (pRemoteSDP->stream[i].conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
				pRemoteSDP->stream[i].conn.pAddress != NULL)
			{
				if ((pRemoteSDP->stream[i].conn.eAddressType != pPrevSDP->stream[i].conn.eAddressType) ||
					(pal_StringCompare(pRemoteSDP->stream[i].conn.pAddress, pPrevSDP->stream[i].conn.pAddress) != 0))
				{
					SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tRemote IP address in c= line has been changed.", __FUNCTION__, __LINE__);
					pInfo->eEvent = ECRIO_SDP_EVENT_IP_ADDRESS_CHANGED;
				}

				break;
			}
		}
	}

	/** Check attribute */
	for (i = 0; i < pRemoteSDP->uNumOfMedia; ++i)
	{
		if (pRemoteSDP->stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
		{
			/** Check media direction */
			if ((pRemoteSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_SENDONLY ||
				pRemoteSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_RECVONLY ||
				pRemoteSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_INACTIVE) &&
				pPrevSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_SENDRECV)
			{
				SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tCall held event.", __FUNCTION__, __LINE__);
				pInfo->eEvent = ECRIO_SDP_EVENT_HOLD;
			}
			else if (pRemoteSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_SENDRECV &&
				(pPrevSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_SENDONLY ||
				pPrevSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_RECVONLY ||
				pPrevSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_INACTIVE))
			{
				SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tCall unhold event.", __FUNCTION__, __LINE__);
				pInfo->eEvent = ECRIO_SDP_EVENT_UNHOLD;
			}
			else if (pRemoteSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_INACTIVE &&
				pPrevSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_RECVONLY)
			{
				SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tCall held on hold event.", __FUNCTION__, __LINE__);
				pInfo->eEvent = ECRIO_SDP_EVENT_INACTIVE;
			}
			else if (pRemoteSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_RECVONLY &&
				pPrevSDP->stream[i].eDirection == ECRIO_SDP_MEDIA_DIRECTION_INACTIVE)
			{
				SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tCall resume event from hold_held state.", __FUNCTION__, __LINE__);
				pInfo->eEvent = ECRIO_SDP_EVENT_UNHOLD;
			}
		}
		else if (pRemoteSDP->stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
		{
			/** Check whether this SDP means a consecutive file transfer or not */
			if (pRemoteSDP->stream[i].u.msrp.fileSelector.pName != NULL ||
				pRemoteSDP->stream[i].u.msrp.fileSelector.uSize != 0 ||
				pRemoteSDP->stream[i].u.msrp.fileSelector.pType != NULL ||
				pRemoteSDP->stream[i].u.msrp.fileSelector.pHash != NULL)
			{
				SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tConsecutive file transfer event.", __FUNCTION__, __LINE__);
				pInfo->eEvent = ECRIO_SDP_EVENT_CONSECUTIVE_FILE_TRANSER;
			}
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

u_int32 ec_SDP_CopyStreamStructure
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pDest,
	EcrioSDPStreamStruct *pSource
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pDest->eMediaType = pSource->eMediaType;
	pDest->uMediaPort = pSource->uMediaPort;
	pDest->eProtocol = pSource->eProtocol;

	pDest->uNumOfPayloads = pSource->uNumOfPayloads;
	if (pSource->uNumOfPayloads > 0)
	{
		for (i = 0; i < pSource->uNumOfPayloads; ++i)
		{
			pDest->payload[i].uType = pSource->payload[i].uType;
			pDest->payload[i].eCodec = pSource->payload[i].eCodec;
			if (pSource->eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
			{
				ec_SDP_CopyAudioPayload(&(pDest->payload[i].u.audio), &(pSource->payload[i].u.audio));
			}
		}
	}

	ec_SDP_CopyBandwidth(&(pDest->bandwidth), &(pSource->bandwidth));
	ec_SDP_CopyPrecondition(&(pDest->precond), &(pSource->precond));

	if (pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4 ||
		pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6)
	{
		pDest->conn.eAddressType = pSource->conn.eAddressType;
	}
	else
	{
		pDest->conn.eAddressType = ECRIO_SDP_ADDRESS_TYPE_NONE;
	}

	if ((pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4 ||
		pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6) &&
		pSource->conn.pAddress != NULL)
	{
		uSDPError = ec_sdp_StringCopy(pStrings,
			(void **)&pDest->conn.pAddress,
			pSource->conn.pAddress,
			pal_StringLength(pSource->conn.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else
	{
		pDest->conn.pAddress = NULL;
	}

	pDest->eDirection = pSource->eDirection;

	switch (pSource->eMediaType)
	{
		case ECRIO_SDP_MEDIA_TYPE_AUDIO:
		{
			pDest->u.audio.uPtime = pSource->u.audio.uPtime;
			pDest->u.audio.uMaxPtime = pSource->u.audio.uMaxPtime;
		}
		break;

		case ECRIO_SDP_MEDIA_TYPE_MSRP:
		{
			pDest->u.msrp.eAcceptTypes = pSource->u.msrp.eAcceptTypes;
			pDest->u.msrp.eAcceptWrappedTypes = pSource->u.msrp.eAcceptWrappedTypes;

			if (pSource->u.msrp.pPath != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.pPath,
					pSource->u.msrp.pPath,
					pal_StringLength(pSource->u.msrp.pPath));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.pPath = NULL;
			}

			pDest->u.msrp.eSetup = pSource->u.msrp.eSetup;

			if (pSource->u.msrp.fileSelector.pName != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.fileSelector.pName,
					pSource->u.msrp.fileSelector.pName,
					pal_StringLength(pSource->u.msrp.fileSelector.pName));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.fileSelector.pName = NULL;
			}

			pDest->u.msrp.fileSelector.uSize = pSource->u.msrp.fileSelector.uSize;

			if (pSource->u.msrp.fileSelector.pType != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.fileSelector.pType,
					pSource->u.msrp.fileSelector.pType,
					pal_StringLength(pSource->u.msrp.fileSelector.pType));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.fileSelector.pType = NULL;
			}

			if (pSource->u.msrp.fileSelector.pHash != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.fileSelector.pHash,
					pSource->u.msrp.fileSelector.pHash,
					pal_StringLength(pSource->u.msrp.fileSelector.pHash));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.fileSelector.pHash = NULL;
			}

			if (pSource->u.msrp.pFileDisposition != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.pFileDisposition,
					pSource->u.msrp.pFileDisposition,
					pal_StringLength(pSource->u.msrp.pFileDisposition));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.pFileDisposition = NULL;
			}

			if (pSource->u.msrp.pFileTransferId != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.pFileTransferId,
					pSource->u.msrp.pFileTransferId,
					pal_StringLength(pSource->u.msrp.pFileTransferId));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.pFileTransferId = NULL;
			}

			if (pSource->u.msrp.fileDate.pCreation != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.fileDate.pCreation,
					pSource->u.msrp.fileDate.pCreation,
					pal_StringLength(pSource->u.msrp.fileDate.pCreation));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.fileDate.pCreation = NULL;
			}

			if (pSource->u.msrp.fileDate.pModification != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.fileDate.pModification,
					pSource->u.msrp.fileDate.pModification,
					pal_StringLength(pSource->u.msrp.fileDate.pModification));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.fileDate.pModification = NULL;
			}

			if (pSource->u.msrp.fileDate.pRead != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.fileDate.pRead,
					pSource->u.msrp.fileDate.pRead,
					pal_StringLength(pSource->u.msrp.fileDate.pRead));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.fileDate.pRead = NULL;
			}

			if (pSource->u.msrp.pFileIcon != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.pFileIcon,
					pSource->u.msrp.pFileIcon,
					pal_StringLength(pSource->u.msrp.pFileIcon));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.pFileIcon = NULL;
			}

			if (pSource->u.msrp.pFingerPrint != NULL)
			{
				uSDPError = ec_sdp_StringCopy(pStrings,
					(void **)&pDest->u.msrp.pFingerPrint,
					pSource->u.msrp.pFingerPrint,
					pal_StringLength(pSource->u.msrp.pFingerPrint));
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}
			else
			{
				pDest->u.msrp.pFingerPrint = NULL;
			}

			pDest->u.msrp.fileRange.uStart = pSource->u.msrp.fileRange.uStart;
			pDest->u.msrp.fileRange.uStop = pSource->u.msrp.fileRange.uStop;
			pDest->u.msrp.bCEMA = pSource->u.msrp.bCEMA;
			pDest->u.msrp.bIsClosed = pSource->u.msrp.bIsClosed;
		}
		break;

		default:
		{
		}
		break;
	}

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

u_int32 ec_SDP_CopySessionStructure
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPSessionStruct *pDest,
	EcrioSDPSessionStruct *pSource
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Reset strings buffer */
	pDest->strings.uSize = 0;

	/** Copy the originator information */
	if (pSource->origin.pUsername != NULL)
	{
		uSDPError = ec_sdp_StringCopy(&pDest->strings,
			(void **)&pDest->origin.pUsername,
			pSource->origin.pUsername,
			pal_StringLength(pSource->origin.pUsername));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else
	{
		pDest->origin.pUsername = NULL;
	}

	pDest->origin.uSessionid = pSource->origin.uSessionid;
	pDest->origin.uVersion = pSource->origin.uVersion;
	pDest->origin.eAddressType = pSource->origin.eAddressType;
	if (pSource->origin.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4 ||
		pSource->origin.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6)
	{
		pDest->origin.eAddressType = pSource->origin.eAddressType;
	}
	else
	{
		pDest->origin.eAddressType = ECRIO_SDP_ADDRESS_TYPE_NONE;
	}

	if ((pSource->origin.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4 ||
		pSource->origin.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6) &&
		pSource->origin.pAddress != NULL)
	{
		uSDPError = ec_sdp_StringCopy(&pDest->strings,
			(void **)&pDest->origin.pAddress,
			pSource->origin.pAddress,
			pal_StringLength(pSource->origin.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else
	{
		pDest->origin.pAddress = NULL;
	}

	/** Copy the connection information */
	if (pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4 ||
		pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6)
	{
		pDest->conn.eAddressType = pSource->conn.eAddressType;
	}
	else
	{
		pDest->conn.eAddressType = ECRIO_SDP_ADDRESS_TYPE_NONE;
	}

	if ((pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4 ||
		pSource->conn.eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6) &&
		pSource->conn.pAddress != NULL)
	{
		uSDPError = ec_sdp_StringCopy(&pDest->strings,
			(void **)&pDest->conn.pAddress,
			pSource->conn.pAddress,
			pal_StringLength(pSource->conn.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else
	{
		pDest->conn.pAddress = NULL;
	}

	ec_SDP_CopyBandwidth(&(pDest->bandwidth), &(pSource->bandwidth));

	/** Copy all media information */
	pDest->uNumOfMedia = pSource->uNumOfMedia;

	for (i = 0; i < pSource->uNumOfMedia; ++i)
	{
		uSDPError = ec_SDP_CopyStreamStructure(S,
			&pDest->strings,
			&pDest->stream[i],
			&pSource->stream[i]);
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
				__FUNCTION__, __LINE__, i, uSDPError);
			goto END;
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

void ec_SDP_InitializeSessionStruct
(
	EcrioSDPSessionStruct *pSDPStruct
)
{
	u_int32 i;

	/** Initialize EcrioSDPSessionStruct without "string" structure. */
	pal_MemorySet(&pSDPStruct->origin, 0, sizeof(EcrioSDPOriginStruct));
	pal_MemorySet(&pSDPStruct->conn, 0, sizeof(EcrioSDPConnectionInfomationStruct));
	pal_MemorySet(&pSDPStruct->bandwidth, 0, sizeof(EcrioSDPBandwidthStruct));
	pSDPStruct->uNumOfMedia = 0;
	for (i = 0; i < MAX_MEDIA; ++i)
	{
		pal_MemorySet(&pSDPStruct->stream[i], 0, sizeof(EcrioSDPStreamStruct));
	}
}

u_int32 ec_SDP_FillLocalIpAddrInfo
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo,
	BoolEnum bIsUseLocalInfo
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_char cName[MAX_USERNAME_LENGTH] = {0};

	s = S->s;

	if (bIsUseLocalInfo == Enum_TRUE)
	{
		s->strings.uSize = 0;

		uSDPError = ec_sdp_StringCopy(&s->strings,
			(void **)&s->pUsername,
			S->localSession.origin.pUsername,
			pal_StringLength(S->localSession.origin.pUsername));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tUser name copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else
	{
		if (NULL == pal_StringNCopy(cName, MAX_USERNAME_LENGTH, (u_char *)s->pUsername, pal_StringLength(s->pUsername)))
		{
			uSDPError = ECRIO_SDP_MEMORY_ERROR;
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}

		s->strings.uSize = 0;

		uSDPError = ec_sdp_StringCopy(&s->strings,
			(void **)&s->pUsername,
			cName,
			pal_StringLength(cName));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tUser name copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}

	s->ip.eAddressType = pInfo->localIp.eAddressType;
	uSDPError = ec_sdp_StringCopy(&s->strings,
		(void **)&s->ip.pAddress,
		pInfo->localIp.pAddress,
		pal_StringLength(pInfo->localIp.pAddress));
	if (uSDPError != ECRIO_SDP_NO_ERROR)
	{
		SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tIP address copy error. uSDPError = %u",
			__FUNCTION__, __LINE__, uSDPError);
		goto END;
	}

END:
	return uSDPError;
}

u_int32 ec_SDP_FillRemoteIpAddrInfo
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;

	s = S->s;

	if (S->remoteSession.stream[0].conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE)
	{
		/** Use the IP from the connection line in each media stream.
		  * For now, use connection line in the first stream. */
		pInfo->remoteIp.eAddressType = S->remoteSession.stream[0].conn.eAddressType;
		uSDPError = ec_sdp_StringCopy(&pInfo->strings,
			(void **)&pInfo->remoteIp.pAddress,
			S->remoteSession.stream[0].conn.pAddress,
			pal_StringLength(S->remoteSession.stream[0].conn.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else if (S->remoteSession.conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE)
	{
		/** There is no connection line in each stream, then use the IP from
		  * the global connection line. */
		pInfo->remoteIp.eAddressType = S->remoteSession.conn.eAddressType;
		uSDPError = ec_sdp_StringCopy(&pInfo->strings,
			(void **)&pInfo->remoteIp.pAddress,
			S->remoteSession.conn.pAddress,
			pal_StringLength(S->remoteSession.conn.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else
	{
		/** Actually, there is an SDP syntax error if either connection line does not
		  * exist on the global session level or each media. Use the IP from the origin
		  * line in an expedient manner. */
		pInfo->remoteIp.eAddressType = S->remoteSession.origin.eAddressType;
		uSDPError = ec_sdp_StringCopy(&pInfo->strings,
			(void **)&pInfo->remoteIp.pAddress,
			S->remoteSession.origin.pAddress,
			pal_StringLength(S->remoteSession.origin.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}

END:
	return uSDPError;
}

u_int32 ec_SDP_UpdateLocalSDPStructure
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPSessionStruct *pLocalSDP = NULL;
	EcrioSDPSessionStruct *pWorkingSDP = NULL;
	EcrioSDPStreamStruct *pStream = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i, j;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pLocalSDP = &S->localSession;
	pWorkingSDP = &S->work;

	pWorkingSDP->strings.uSize = 0;

	/** Copy local SDP to the working buffer */
	uSDPError = ec_SDP_CopySessionStructure(S,
		&S->work,
		&S->localSession);
	if (uSDPError != ECRIO_SDP_NO_ERROR)
	{
		SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tSession information copy error. uSDPError = %u",
			__FUNCTION__, __LINE__, uSDPError);
		goto END;
	}

	/** If event is identical SDP, there is no local SDP to be updated */
	if (pInfo->eEvent == ECRIO_SDP_EVENT_IDENTICAL)
	{
		SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\teEvent is ECRIO_SDP_EVENT_IDENTICAL, so there is no SDP to update",
			__FUNCTION__, __LINE__);
		goto END;
	}

	pLocalSDP->strings.uSize = 0;

	/** Copy the originator information */
	if (pWorkingSDP->origin.pUsername != NULL)
	{
		uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
			(void **)&pLocalSDP->origin.pUsername,
			pWorkingSDP->origin.pUsername,
			pal_StringLength(pWorkingSDP->origin.pUsername));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}
	else
	{
		pLocalSDP->origin.pUsername = NULL;
	}

	pLocalSDP->origin.uSessionid = pWorkingSDP->origin.uSessionid;
	pLocalSDP->origin.uVersion = pWorkingSDP->origin.uVersion;

	/** Check whether the local IP address has been changed */
	if (pInfo->eEvent == ECRIO_SDP_EVENT_IP_ADDRESS_CHANGED &&
		(pInfo->localIp.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
		pInfo->localIp.pAddress != NULL))
	{
		if ((pInfo->localIp.eAddressType != pWorkingSDP->origin.eAddressType) ||
			(pal_StringCompare(pInfo->localIp.pAddress, pWorkingSDP->origin.pAddress) != 0))
		{
			pLocalSDP->origin.eAddressType = pInfo->localIp.eAddressType;
			uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
				(void **)&pLocalSDP->origin.pAddress,
				pInfo->localIp.pAddress,
				pal_StringLength(pInfo->localIp.pAddress));
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Update local memory */
			uSDPError = ec_SDP_FillLocalIpAddrInfo(S, pInfo, Enum_TRUE);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFilling local IP address infomation error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}
		}
	}
	else
	{
		pLocalSDP->origin.eAddressType = pWorkingSDP->origin.eAddressType;
		if (pWorkingSDP->origin.pAddress != NULL)
		{
			uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
				(void **)&pLocalSDP->origin.pAddress,
				pWorkingSDP->origin.pAddress,
				pal_StringLength(pWorkingSDP->origin.pAddress));
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}
		}
		else
		{
			pLocalSDP->origin.pAddress = NULL;
		}
	}

	/** Copy the connection information */

	/** Check whether the local IP address has been changed */
	if (pInfo->eEvent == ECRIO_SDP_EVENT_IP_ADDRESS_CHANGED &&
		(pInfo->localIp.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
		pInfo->localIp.pAddress != NULL))
	{
		if ((pInfo->localIp.eAddressType != pWorkingSDP->conn.eAddressType) ||
			(pal_StringCompare(pInfo->localIp.pAddress, pWorkingSDP->conn.pAddress) != 0))
		{
			pLocalSDP->conn.eAddressType = pInfo->localIp.eAddressType;
			uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
				(void **)&pLocalSDP->conn.pAddress,
				pInfo->localIp.pAddress,
				pal_StringLength(pInfo->localIp.pAddress));
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}
		}
	}
	else
	{
		pLocalSDP->conn.eAddressType = pWorkingSDP->conn.eAddressType;
		if (pWorkingSDP->conn.pAddress != NULL)
		{
			uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
				(void **)&pLocalSDP->conn.pAddress,
				pWorkingSDP->conn.pAddress,
				pal_StringLength(pWorkingSDP->conn.pAddress));
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}
		}
		else
		{
			pLocalSDP->conn.pAddress = NULL;
		}
	}

	ec_SDP_CopyBandwidth(&(pLocalSDP->bandwidth), &(pWorkingSDP->bandwidth));

	/** Copy the media information */
	pLocalSDP->uNumOfMedia = pWorkingSDP->uNumOfMedia;

	for (i = 0; i < pWorkingSDP->uNumOfMedia; ++i)
	{
		if (pWorkingSDP->stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
		{
			uSDPError = ec_SDP_CopyStreamStructure(S,
				&pLocalSDP->strings,
				&pLocalSDP->stream[i],
				&pWorkingSDP->stream[i]);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
					__FUNCTION__, __LINE__, i, uSDPError);
				goto END;
			}

#if 1
			if (pInfo->eEvent == ECRIO_SDP_EVENT_HOLD || pInfo->eEvent == ECRIO_SDP_EVENT_UNHOLD)
			{
				/** Note: when the upper layer handles direction attribute for hold/unhold call,
				  * it may be enabled this statement. */
				for (j = 0; j < pInfo->uNumOfLocalMedia; j++)
				{
					if (pInfo->localStream[j].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
					{
						pStream = &pInfo->localStream[j];
					}
				}

				if (pStream == NULL)
				{
					uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tThere is no audio stream information in input format, uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}

				pLocalSDP->stream[i].eDirection = pStream->eDirection;
			}
#else
			if (pInfo->eEvent == ECRIO_SDP_EVENT_HOLD)
			{
				pLocalSDP->stream[i].eDirection = ECRIO_SDP_MEDIA_DIRECTION_INACTIVE;
			}
			else if (pInfo->eEvent == ECRIO_SDP_EVENT_UNHOLD)
			{
				pLocalSDP->stream[i].eDirection = ECRIO_SDP_MEDIA_DIRECTION_SENDRECV;
			}
#endif
		}
		else if (pWorkingSDP->stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
		{
			if (pInfo->eEvent == ECRIO_SDP_EVENT_CONSECUTIVE_FILE_TRANSER)
			{
				for (j = 0; j < pInfo->uNumOfLocalMedia; j++)
				{
					if (pInfo->localStream[j].eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
					{
						pStream = &pInfo->localStream[j];
					}
				}

				if (pStream == NULL)
				{
					uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tThere is no MSRP stream information in input format, uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}

				pLocalSDP->stream[i].u.msrp.eAcceptTypes = pStream->u.msrp.eAcceptTypes;
				pLocalSDP->stream[i].u.msrp.eAcceptWrappedTypes = pStream->u.msrp.eAcceptWrappedTypes;

				if (pStream->u.msrp.pPath != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.pPath,
						pStream->u.msrp.pPath,
						pal_StringLength(pStream->u.msrp.pPath));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.pPath = NULL;
				}

				pLocalSDP->stream[i].u.msrp.eSetup = pStream->u.msrp.eSetup;

				if (pStream->u.msrp.fileSelector.pName != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.fileSelector.pName,
						pStream->u.msrp.fileSelector.pName,
						pal_StringLength(pStream->u.msrp.fileSelector.pName));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.fileSelector.pName = NULL;
				}

				pLocalSDP->stream[i].u.msrp.fileSelector.uSize = pStream->u.msrp.fileSelector.uSize;

				if (pStream->u.msrp.fileSelector.pType != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.fileSelector.pType,
						pStream->u.msrp.fileSelector.pType,
						pal_StringLength(pStream->u.msrp.fileSelector.pType));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.fileSelector.pType = NULL;
				}

				if (pStream->u.msrp.fileSelector.pHash != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.fileSelector.pHash,
						pStream->u.msrp.fileSelector.pHash,
						pal_StringLength(pStream->u.msrp.fileSelector.pHash));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.fileSelector.pHash = NULL;
				}

				if (pStream->u.msrp.pFileDisposition != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.pFileDisposition,
						pStream->u.msrp.pFileDisposition,
						pal_StringLength(pStream->u.msrp.pFileDisposition));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.pFileDisposition = NULL;
				}

				if (pStream->u.msrp.pFileTransferId != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.pFileTransferId,
						pStream->u.msrp.pFileTransferId,
						pal_StringLength(pStream->u.msrp.pFileTransferId));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.pFileTransferId = NULL;
				}

				if (pStream->u.msrp.fileDate.pCreation != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.fileDate.pCreation,
						pStream->u.msrp.fileDate.pCreation,
						pal_StringLength(pStream->u.msrp.fileDate.pCreation));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.fileDate.pCreation = NULL;
				}

				if (pStream->u.msrp.fileDate.pModification != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.fileDate.pModification,
						pStream->u.msrp.fileDate.pModification,
						pal_StringLength(pStream->u.msrp.fileDate.pModification));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.fileDate.pModification = NULL;
				}

				if (pStream->u.msrp.fileDate.pRead != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.fileDate.pRead,
						pStream->u.msrp.fileDate.pRead,
						pal_StringLength(pStream->u.msrp.fileDate.pRead));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.fileDate.pRead = NULL;
				}

				if (pStream->u.msrp.pFileIcon != NULL)
				{
					uSDPError = ec_sdp_StringCopy(&pLocalSDP->strings,
						(void **)&pLocalSDP->stream[i].u.msrp.pFileIcon,
						pStream->u.msrp.pFileIcon,
						pal_StringLength(pStream->u.msrp.pFileIcon));
					if (uSDPError != ECRIO_SDP_NO_ERROR)
					{
						SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
							__FUNCTION__, __LINE__, uSDPError);
						goto END;
					}
				}
				else
				{
					pLocalSDP->stream[i].u.msrp.pFileIcon = NULL;
				}

				pLocalSDP->stream[i].u.msrp.fileRange.uStart = pStream->u.msrp.fileRange.uStart;
				pLocalSDP->stream[i].u.msrp.fileRange.uStop = pStream->u.msrp.fileRange.uStop;
				pLocalSDP->stream[i].u.msrp.bCEMA = pStream->u.msrp.bCEMA;
				pLocalSDP->stream[i].u.msrp.bIsClosed = pStream->u.msrp.bIsClosed;
			}
			else
			{
				uSDPError = ec_SDP_CopyStreamStructure(S,
					&pLocalSDP->strings,
					&pLocalSDP->stream[i],
					&pWorkingSDP->stream[i]);
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
						__FUNCTION__, __LINE__, i, uSDPError);
					goto END;
				}
			}
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

u_int32 ec_SDP_FillNegotiatedMediaInfo
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i;

	s = S->s;

	pInfo->uNumOfLocalMedia = S->localSession.uNumOfMedia;

	for (i = 0; i < S->localSession.uNumOfMedia; ++i)
	{
		uSDPError = ec_SDP_CopyStreamStructure( S,
			&pInfo->strings,
			&pInfo->localStream[i],
			&S->localSession.stream[i]);
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tLocal stream information copy error. i = %d, uSDPError = %u",
				__FUNCTION__, __LINE__, i, uSDPError);
			goto END;
		}
	}

	pInfo->uNumOfRemoteMedia = S->remoteSession.uNumOfMedia;

	for (i = 0; i < S->remoteSession.uNumOfMedia; ++i)
	{
		uSDPError = ec_SDP_CopyStreamStructure( S,
			&pInfo->strings,
			&pInfo->remoteStream[i],
			&S->remoteSession.stream[i]);
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tRemote stream information copy error. i = %d, uSDPError = %u",
				__FUNCTION__, __LINE__, i, uSDPError);
			goto END;
		}
	}

END:
	return uSDPError;
}

u_int32 ec_SDP_FillSDPMessageBody
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPSessionStruct *pSession,
	u_int8 uNumOfList,
	EcrioSDPStreamStruct *pStream,
	EcrioSDPEventTypeEnum eEvent
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPStreamStruct *pDest = NULL;
	EcrioSDPStreamStruct *pSource = NULL;
	EcrioSDPStreamStruct *pRemote = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i, j;
	s_char LocalAudioIndex = -1;
	s_char RemoteAudioIndex = -1;
	s_char LocalDTMFIndex = -1;
	s_char RemoteDTMFIndex = -1;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** First we fill the local session struct, then next copy the content of
	  * local sesstion struct to specified (pSession) buffer. */

	S->localSession.strings.uSize = 0;

	/** Fill Origin struct */
	{
		/** User name */
		uSDPError = ec_sdp_StringCopy(&S->localSession.strings,
			(void **)&S->localSession.origin.pUsername,
			s->pUsername,
			pal_StringLength(s->pUsername));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}

		pSession->origin.pUsername = S->localSession.origin.pUsername;

		if ((S->eState == ECRIO_SDP_State_None && S->ePrecondState == ECRIO_SDP_PRECONDITION_STATE_NONE) || 
			(S->eState == ECRIO_SDP_State_InitialRemoteOffer && S->ePrecondState <= ECRIO_SDP_PRECONDITION_STATE_INITIATE))
		{
			/** Session ID */
			S->localSession.origin.uSessionid = ec_sdp_GetNTPtime();
			pSession->origin.uSessionid = S->localSession.origin.uSessionid;

			/** Version */
			S->localSession.origin.uVersion = S->localSession.origin.uSessionid;
			pSession->origin.uVersion = S->localSession.origin.uVersion;
		}
		else
		{
			/** Already the local session struct was filled, then copy the content
			  * of local sesstion struct to pSession. */

			/** Session ID */
			pSession->origin.uSessionid = S->localSession.origin.uSessionid;

			/** Version -- need to increment except identical SDP */
			if (eEvent != ECRIO_SDP_EVENT_IDENTICAL)
			{
				S->localSession.origin.uVersion++;
			}
			pSession->origin.uVersion = S->localSession.origin.uVersion;
		}

		/** Address type */
		S->localSession.origin.eAddressType = s->ip.eAddressType;
		pSession->origin.eAddressType = S->localSession.origin.eAddressType;

		/** IP address */
		uSDPError = ec_sdp_StringCopy(&S->localSession.strings,
			(void **)&S->localSession.origin.pAddress,
			s->ip.pAddress,
			pal_StringLength(s->ip.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}

		pSession->origin.pAddress = S->localSession.origin.pAddress;
	}

	/** Fill Connection Information struct */
	{
		/** Address type */
		S->localSession.conn.eAddressType = s->ip.eAddressType;
		pSession->conn.eAddressType = S->localSession.conn.eAddressType;

		/** IP address */
		uSDPError = ec_sdp_StringCopy(&S->localSession.strings,
			(void **)&S->localSession.conn.pAddress,
			s->ip.pAddress,
			pal_StringLength(s->ip.pAddress));
		if (uSDPError != ECRIO_SDP_NO_ERROR)
		{
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tString copy error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}

		pSession->conn.pAddress = S->localSession.conn.pAddress;
	}

	/** Fill global bandwidth struct */
	{
		/**  Note: need to calculate global bandwidth value when media description has
		  *  two or more "b=" line in those media sessions, e.g. audio and video.
		  */
		/** For now, set bandwidth value from first stream. */
		ec_SDP_CopyBandwidth(&(pSession->bandwidth), &(pStream[0].bandwidth));
	}

	/** Fill stream struct */
	{
		/** Number of media */
		S->localSession.uNumOfMedia = uNumOfList;

		/** Fill stream struct to local */
		for (i = 0; i < uNumOfList; ++i)
		{
			uSDPError = ec_SDP_CopyStreamStructure(S,
				&S->localSession.strings,
				&S->localSession.stream[i],
				&pStream[i]);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
					__FUNCTION__, __LINE__, i, uSDPError);
				goto END;
			}
		}

		/** Copy local stored data to the specified buffer */
		pSession->uNumOfMedia = uNumOfList;

		for (i = 0; i < uNumOfList; ++i)
		{
			pDest = &pSession->stream[i];
			pSource = &S->localSession.stream[i];
			pRemote = &S->remoteSession.stream[i];

			pDest->eMediaType = pSource->eMediaType;
			pDest->uMediaPort = pSource->uMediaPort;
			pDest->eProtocol = pSource->eProtocol;

			pDest->uNumOfPayloads = pSource->uNumOfPayloads;
			if (pSource->uNumOfPayloads > 0)
			{
				for (j = 0; j < pSource->uNumOfPayloads; j++)
				{
					pDest->payload[j].uType = pSource->payload[j].uType;
					pDest->payload[j].eCodec = pSource->payload[j].eCodec;
					if (pSource->eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
					{
						/** If SDP state is offer, we add local parameters for audio which provided from upper layer.
						  * Otherwise SDP state is answer, then use remote parameters indicated by offer that has been negotiated. */
						switch (S->eState)
						{
							/** Initial offer / subsequent offer */
							case ECRIO_SDP_State_None:
							case ECRIO_SDP_State_Negotiated:
							{
								ec_SDP_CopyAudioPayload(&(pDest->payload[j].u.audio), &(pSource->payload[j].u.audio));
							}
							break;

							/** Initial answer / subsequent answer */
							default:
							{
								ec_SDP_FindMatchAudioPayloads(S,
									pSource,
									pRemote,
									&LocalAudioIndex,
									&RemoteAudioIndex,
									&LocalDTMFIndex,
									&RemoteDTMFIndex);

								if (pSource->payload[j].eCodec != EcrioAudioCodec_DTMF && RemoteAudioIndex != -1)
								{
									ec_SDP_CopyAudioPayload(&(pDest->payload[j].u.audio), &(pRemote->payload[RemoteAudioIndex].u.audio));
								}
								else if (pSource->payload[j].eCodec == EcrioAudioCodec_DTMF && RemoteDTMFIndex != -1)
								{
									ec_SDP_CopyAudioPayload(&(pDest->payload[j].u.audio), &(pRemote->payload[RemoteDTMFIndex].u.audio));
								}
								else
								{
									SDPLOGW(s->logHandle, KLogTypeGeneral, "%s:%u\tThere is no SDP offer...??? then set local paramters.",
										__FUNCTION__, __LINE__, i, uSDPError);

									ec_SDP_CopyAudioPayload(&(pDest->payload[j].u.audio), &(pSource->payload[j].u.audio));
								}
							}
							break;
						}
					}
				}
			}

			ec_SDP_CopyBandwidth(&(pDest->bandwidth), &(pSource->bandwidth));
			ec_SDP_CopyPrecondition(&(pDest->precond), &(pSource->precond));

			pDest->conn.eAddressType = pSource->conn.eAddressType;
			pDest->conn.pAddress = pSource->conn.pAddress;
			pDest->eDirection = pSource->eDirection;

			switch (pSource->eMediaType)
			{
				case ECRIO_SDP_MEDIA_TYPE_AUDIO:
				{
					pDest->u.audio.uPtime = pSource->u.audio.uPtime;
					pDest->u.audio.uMaxPtime = pSource->u.audio.uMaxPtime;
				}
				break;

				case ECRIO_SDP_MEDIA_TYPE_MSRP:
				{
					pDest->u.msrp.eAcceptTypes = pSource->u.msrp.eAcceptTypes;
					pDest->u.msrp.eAcceptWrappedTypes = pSource->u.msrp.eAcceptWrappedTypes;
					pDest->u.msrp.pPath = pSource->u.msrp.pPath;
					pDest->u.msrp.eSetup = pSource->u.msrp.eSetup;
					pDest->u.msrp.fileSelector.pName = pSource->u.msrp.fileSelector.pName;
					pDest->u.msrp.fileSelector.uSize = pSource->u.msrp.fileSelector.uSize;
					pDest->u.msrp.fileSelector.pType = pSource->u.msrp.fileSelector.pType;
					pDest->u.msrp.fileSelector.pHash = pSource->u.msrp.fileSelector.pHash;
					pDest->u.msrp.pFileDisposition = pSource->u.msrp.pFileDisposition;
					pDest->u.msrp.pFileTransferId = pSource->u.msrp.pFileTransferId;
					pDest->u.msrp.fileDate.pCreation = pSource->u.msrp.fileDate.pCreation;
					pDest->u.msrp.fileDate.pModification = pSource->u.msrp.fileDate.pModification;
					pDest->u.msrp.fileDate.pRead = pSource->u.msrp.fileDate.pRead;
					pDest->u.msrp.pFileIcon = pSource->u.msrp.pFileIcon;
					pDest->u.msrp.fileRange.uStart = pSource->u.msrp.fileRange.uStart;
					pDest->u.msrp.fileRange.uStop = pSource->u.msrp.fileRange.uStop;
					pDest->u.msrp.bCEMA = pSource->u.msrp.bCEMA;
					pDest->u.msrp.bIsClosed = pSource->u.msrp.bIsClosed;
				}
				break;

				default:
				{
				}
				break;
			}
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

u_int32 ec_SDP_CheckMediaStatus
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo
)
{
	EcrioSDPStruct *s = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pInfo->uNumOfLocalMedia == 0)
	{
		uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
		SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tSpecified no media. uSDPError = %u",
			__FUNCTION__, __LINE__, uSDPError);
		goto END;
	}

	for (i = 0; i < pInfo->uNumOfLocalMedia; ++i)
	{
		/** Check media connection information for media or global line */
		if (pInfo->localStream[i].conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE)
		{
			if (pInfo->localStream[i].conn.eAddressType != s->ip.eAddressType)
			{
				uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid IP address type in media connection information. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}
		}
		else if (pInfo->localIp.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE)
		{
			if (pInfo->localIp.eAddressType != s->ip.eAddressType)
			{
				uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid IP address type in global connection information. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}
		}

		/** Check media protocol */
		if (pInfo->localStream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
		{
			if (pInfo->localStream[i].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_RTP_AVP)
			{
				uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid media protocol in audio stream (%d). uSDPError = %u",
					__FUNCTION__, __LINE__, pInfo->localStream[i].eProtocol, uSDPError);
				goto END;
			}
		}
		else if (pInfo->localStream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
		{
			if (pInfo->localStream[i].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP &&
				pInfo->localStream[i].eProtocol != ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP)
			{
				uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid media protocol in MSRP session (%d). uSDPError = %u",
					__FUNCTION__, __LINE__, pInfo->localStream[i].eProtocol, uSDPError);
				goto END;
			}
		}
		else
		{
			uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid media type: i = %d, media type = %d,  uSDPError = %u",
				__FUNCTION__, __LINE__, i, pInfo->localStream[i].eMediaType, uSDPError);
			goto END;
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

/** Structure to abstract parameter list for the function:
    ec_SDP_VerifyAudioAMRPayloadToMatch. */
typedef struct
{
	EcrioSDPInternalSessionStruct *S;
	EcrioSDPPayloadStruct *pLocal;
	EcrioSDPPayloadStruct *pRemote;
	BoolEnum bAMRWB;
} EcrioSDPInternalInternalAMRParameterStruct;

/** Handles both AMR and AMR-WB. */
BoolEnum ec_SDP_VerifyAudioAMRPayloadToMatch
(
	EcrioSDPInternalInternalAMRParameterStruct *pParams,
	BoolEnum *pFlag
)
{
	EcrioSDPStruct *s = NULL;
	EcrioAudioCodecAMRStruct tLocalAMRparam = { 0 };
	EcrioAudioCodecAMRStruct tRemoteAMRparam = { 0 };
	EcrioAudioCodecAMRStruct param = { 0 };
	u_int32 uLocalModeSet = 0;
	u_int32 uRemoteModeSet = 0;
	u_int32 uAllOnModeSet = 0;
	BoolEnum bMatched = Enum_FALSE;
	BoolEnum bFlag = Enum_FALSE;

	/** @note This is an internal function, so it is assumed that the pointers are valid. */

	s = pParams->S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Set the default bitmasks for the specific type. */
	if (pParams->bAMRWB == Enum_TRUE)
	{
		/** All bits on for AMR-WB (as mode-set: 0,1,2,3,4,5,6,7,8). */
		uAllOnModeSet = 0x1FF;
	}
	else
	{
		/** All bits on for AMR (as mode-set: 0,1,2,3,4,5,6,7,8). */
		uAllOnModeSet = 0xFF;
	}

	/** Verify AMR/AM-WB payload with local and remote information to match */

	if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pParams->pLocal->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
	{
		SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		goto END;
	}
	tLocalAMRparam.uModeSet = param.uModeSet;
	tLocalAMRparam.uOctetAlign = param.uOctetAlign;
	tLocalAMRparam.uCRC = param.uCRC;
	if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pParams->pRemote->u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
	{
		SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
			__FUNCTION__, __LINE__);
		goto END;
	}
	tRemoteAMRparam.uModeSet = param.uModeSet;
	tRemoteAMRparam.uOctetAlign = param.uOctetAlign;
	tRemoteAMRparam.uCRC = param.uCRC;

	SDPLOGV(s->logHandle, KLogTypeGeneral,
		"%s:%u\t Local params: octet-align=%d, crc=%d - Remote params: octet-align=%d, crc=%d",
		__FUNCTION__, __LINE__, tLocalAMRparam.uOctetAlign, tLocalAMRparam.uCRC, tRemoteAMRparam.uOctetAlign, tRemoteAMRparam.uCRC);

	if (tLocalAMRparam.uOctetAlign != tRemoteAMRparam.uOctetAlign ||
		tLocalAMRparam.uCRC != tRemoteAMRparam.uCRC)
	{
		/** AMR specific paramters are not matched, then return */
		bMatched = Enum_FALSE;
		SDPLOGV(s->logHandle, KLogTypeGeneral, "%s:%u\t  AMR specific paramters are not matched.", __FUNCTION__, __LINE__);
		goto END;
	}

	uLocalModeSet = tLocalAMRparam.uModeSet;
	uRemoteModeSet = tRemoteAMRparam.uModeSet;

	SDPLOGV(s->logHandle, KLogTypeGeneral,
		"%s:%u\t Local mode-set: %d, Remote mode-set: %d",
		__FUNCTION__, __LINE__, uLocalModeSet, uRemoteModeSet);

	/** Exchange mode-set if there is no mode-set restriction (no description) */
	if (uLocalModeSet == 0)
	{
		uLocalModeSet = uAllOnModeSet;
	}

	if (uRemoteModeSet == 0)
	{
		uRemoteModeSet = uAllOnModeSet;
	}

	/** Remote mode-set is unrestricted (full bits on) */
	if (uRemoteModeSet == uAllOnModeSet)
	{
		/** Local mode-set is unrestricted (full bits on) */
		if (uLocalModeSet == uAllOnModeSet)
		{
			/** Both local and remote are no mode-set restriction, then matched */
			SDPLOGV(s->logHandle, KLogTypeGeneral,
				"%s:%u\t  Both local and remote are no mode-set restriction, then matched.",
				__FUNCTION__, __LINE__);
			bMatched = Enum_TRUE;
		}
		else
		{
			/** Local mode-set is restrected and Remote mode-set is unrestricted, then matched.
			  * But it need to re-check overall local codec list. */
			SDPLOGV(s->logHandle, KLogTypeGeneral,
				"%s:%u\t  Local mode-set is restrected and Remote mode-set is unrestricted, then matched. " \
				"But it need to re-check overall local codec list.",
				__FUNCTION__, __LINE__);
			bMatched = Enum_TRUE;

			/** Set flag to mean that need to find "best codec" by checking overall local codec list. */
			bFlag = Enum_TRUE;
		}
	}
	/** Remote mode-set is restricted */
	else
	{
		if (uLocalModeSet == uRemoteModeSet || uLocalModeSet == uAllOnModeSet)
		{
			/** Exactly matched both mode-set or local mode-set is unrestricted (full bits on),
			  * then matched */
			SDPLOGV(s->logHandle, KLogTypeGeneral,
				"%s:%u\t  Exactly matched both mode-set or local mode-set is unrestricted (full bits on), then matched",
				__FUNCTION__, __LINE__);
			bMatched = Enum_TRUE;
		}
	}

END:
	*pFlag = bFlag;

	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return bMatched;
}

void ec_SDP_FindMatchAudioPayloads
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPStreamStruct *pLocal,
	EcrioSDPStreamStruct *pRemote,
	s_char *pLocalAudioIndex,
	s_char *pRemoteAudioIndex,
	s_char *pLocalDTMFIndex,
	s_char *pRemoteDTMFIndex
)
{
	EcrioSDPStruct *s = NULL;
	u_int8 uLpCnt = 0;
	u_int8 uLptCnt = 0;
	u_int8 uRpCnt = 0;
	u_int8 uTmp = 0;
	u_int8 uLocalChannels = 0;
	u_int8 uRemoteChannels = 0;
	s_char LocalAudioIndex = -1;
	s_char RemoteAudioIndex = -1;
	s_char LocalDTMFIndex = -1;
	s_char RemoteDTMFIndex = -1;
	EcrioAudioCodecAMRStruct param = { 0 };
	u_int32 uTempModeSet = 0;
	u_int8 uTopWeight = 0;
	u_int8 uCurrentWeight = 0;
	u_int8 uMaxLoop = 0;
	BoolEnum bMatched = Enum_FALSE;
	BoolEnum bFlag = Enum_FALSE;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Compare remote audio payload with local audio payload */
	for (uRpCnt = 0; uRpCnt < pRemote->uNumOfPayloads; uRpCnt++)
	{
		/** Check payload type */
		if (pRemote->payload[uRpCnt].uType > ECRIO_SDP_CODEC_MAX_DYNAMIC_VALUE)
		{
			/** Exceed max dynamic value */
			SDPLOGV(s->logHandle, KLogTypeGeneral, "%s:%u\tLoop[%d]: payload type exceeds max dynamic value (%d), hence skip this payload type",
				__FUNCTION__, __LINE__, uRpCnt, pRemote->payload[uRpCnt].uType);
			continue;
		}
		else if (pRemote->payload[uRpCnt].uType < ECRIO_SDP_CODEC_MIN_DYNAMIC_VALUE)
		{
			/** Static payload type */
			SDPLOGV(s->logHandle, KLogTypeGeneral,
				"%s:%u\tLoop[%d]: Remote payload type is static (%d), check a same payload type in local codec list.",
				__FUNCTION__, __LINE__, uRpCnt, pRemote->payload[uRpCnt].uType);

			/** Check only a payload type in static payload */
			for (uLpCnt = 0; uLpCnt < pLocal->uNumOfPayloads; uLpCnt++)
			{
				if (pLocal->payload[uLpCnt].uType == pRemote->payload[uRpCnt].uType)
				{
					/** Found matching static payload */
					if (LocalAudioIndex == -1)
					{
						LocalAudioIndex = (char)uLpCnt;
						RemoteAudioIndex = (char)uRpCnt;

						SDPLOGV(s->logHandle, KLogTypeGeneral,
							"%s:%u\tLoop[%d]:  Found matching payload. Local index: %d, Remote index: %d",
							__FUNCTION__, __LINE__, uRpCnt, LocalAudioIndex, RemoteAudioIndex);
					}

					break;
				}
			}
		}
		else
		{
			/** Dynamic payload type */
			SDPLOGV(s->logHandle, KLogTypeGeneral,
				"%s:%u\tLoop[%d]: Remote payload type is dynamic (%d), codec name, sampling rate, and channel count.",
				__FUNCTION__, __LINE__, uRpCnt, pRemote->payload[uRpCnt].uType);

			/** Check codec name, sampling rate, and channel count */
			for (uLpCnt = 0; uLpCnt < pLocal->uNumOfPayloads; uLpCnt++)
			{
				uLocalChannels = pLocal->payload[uLpCnt].u.audio.uChannels;
				uRemoteChannels = pRemote->payload[uRpCnt].u.audio.uChannels;

				/** The value of uChannels '0' means that drops channel part in SDP rtpmap
				  * such as "AMR/8000", but it equals which channel is '1'. Therefore, it consider
				  * the value of uChannels '0' as '1'.  */
				if (uLocalChannels == 0)
				{
					uLocalChannels = 1;
				}

				if (uRemoteChannels == 0)
				{
					uRemoteChannels = 1;
				}

				if (pLocal->payload[uLpCnt].eCodec == pRemote->payload[uRpCnt].eCodec &&
					pLocal->payload[uLpCnt].u.audio.uClockrate == pRemote->payload[uRpCnt].u.audio.uClockrate &&
					uLocalChannels == uRemoteChannels)
				{
					/** Found matching dynamic payload */
					if (pLocal->payload[uLpCnt].eCodec == EcrioAudioCodec_DTMF)
					{
						/** DTMF */
						if (LocalDTMFIndex == -1)
						{
							LocalDTMFIndex = (char)uLpCnt;
							RemoteDTMFIndex = (char)uRpCnt;

							SDPLOGV(s->logHandle, KLogTypeGeneral,
								"%s:%u\tLoop[%d]:  Found matching DTMF payload. Local index: %d, Remote index: %d",
								__FUNCTION__, __LINE__, uRpCnt, LocalDTMFIndex, RemoteDTMFIndex);
						}

						break;
					}
					else if (pLocal->payload[uLpCnt].eCodec == EcrioAudioCodec_AMR ||
						pLocal->payload[uLpCnt].eCodec == EcrioAudioCodec_AMR_WB)
					{
						/** AMR/AMR-WB payload */
						if (LocalAudioIndex == -1)
						{
							EcrioSDPInternalInternalAMRParameterStruct amrMatchParams;

							amrMatchParams.S = S;
							amrMatchParams.pLocal = &pLocal->payload[uLpCnt];
							amrMatchParams.pRemote = &pRemote->payload[uRpCnt];

							if (pLocal->payload[uLpCnt].eCodec == EcrioAudioCodec_AMR)
							{
								amrMatchParams.bAMRWB = Enum_FALSE;
								bMatched = ec_SDP_VerifyAudioAMRPayloadToMatch(&amrMatchParams, &bFlag);
							}
							else
							{
								amrMatchParams.bAMRWB = Enum_TRUE;
								bMatched = ec_SDP_VerifyAudioAMRPayloadToMatch(&amrMatchParams, &bFlag);
							}

							/** Matched local and remote codec? */
							if (bMatched == Enum_FALSE)
							{
								/** Not matched, then continue */
								continue;
							}

							/** Need to find "best match" codec? */
							if (bFlag == Enum_TRUE)
							{
								SDPLOGV(s->logHandle, KLogTypeGeneral,
									"%s:%u\tLoop[%d]: Need to find \"best codec\" by checking overall local codec list.",
									__FUNCTION__, __LINE__, uRpCnt);

								uTopWeight = 0;

								for (uLptCnt = 0; uLptCnt < pLocal->uNumOfPayloads; uLptCnt++)
								{
									if (pLocal->payload[uLptCnt].eCodec == pRemote->payload[uRpCnt].eCodec)
									{
										if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pLocal->payload[uLptCnt].u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
										{
											SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
												__FUNCTION__, __LINE__);
											goto END;
										}
										uTempModeSet = param.uModeSet;

										/** If no mode-set restriction, there is "best match" codec */
										if (uTempModeSet == 0)
										{
											LocalAudioIndex = (char)uLptCnt;
											break;
										}

										/** Compare "match weight" of mode-set, then choose highest weight mode options
										  * If current weight is higher than (stored) top weight then overwrite top weight
										  * by current weight and store current index as matched */
										uCurrentWeight = 0;
										if (pLocal->payload[uLptCnt].eCodec == EcrioAudioCodec_AMR)
										{
											uMaxLoop = 8;
										}
										else
										{
											uMaxLoop = 9;
										}

										for (uTmp = 0; uTmp < uMaxLoop; uTmp++)
										{
											if (((uTempModeSet >> uTmp) & 0x1) != 0)
											{
												uCurrentWeight += uTmp + 1;
											}
										}

										if (uCurrentWeight > uTopWeight)
										{
											uTopWeight = uCurrentWeight;
											LocalAudioIndex = (char)uLptCnt;
										}
									}
								}

								RemoteAudioIndex = (char)uRpCnt;

								SDPLOGV(s->logHandle, KLogTypeGeneral,
									"%s:%u\tLoop[%d]:  Found best matching AMR/AMR-WB payload. Local index: %d, Remote index: %d",
									__FUNCTION__, __LINE__, uRpCnt, LocalAudioIndex, RemoteAudioIndex);
							}
							else
							{
								LocalAudioIndex = (char)uLpCnt;
								RemoteAudioIndex = (char)uRpCnt;

								SDPLOGV(s->logHandle, KLogTypeGeneral,
									"%s:%u\tLoop[%d]:  Found matching AMR/AMR-WB payload. Local index: %d, Remote index: %d",
									__FUNCTION__, __LINE__, uRpCnt, LocalAudioIndex, RemoteAudioIndex);
							}
						}

						break;
					}
				}
			}
		}

		if (LocalAudioIndex != -1 && LocalDTMFIndex != -1)
		{
			break;
		}
	}

	*pLocalAudioIndex = LocalAudioIndex;
	*pRemoteAudioIndex = RemoteAudioIndex;
	*pLocalDTMFIndex = LocalDTMFIndex;
	*pRemoteDTMFIndex = RemoteDTMFIndex;

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
}

u_int32 ec_SDP_FindMatchPayloads
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPInformationStruct *pInfo,
	BoolEnum bAddRemote
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPStreamStruct *pLocal = NULL;
	EcrioSDPStreamStruct *pRemote = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i, j;
	s_char LocalAudioIndex = -1;
	s_char RemoteAudioIndex = -1;
	s_char LocalDTMFIndex = -1;
	s_char RemoteDTMFIndex = -1;
	EcrioAudioCodecAMRStruct param = { 0 };

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pInfo->uNumOfLocalMedia == 0)
	{
		uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
		SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tSpecified no media. uSDPError = %u",
			__FUNCTION__, __LINE__, uSDPError);
		goto END;
	}

	for (i = 0; i < pInfo->uNumOfLocalMedia; ++i)
	{
		pLocal = &pInfo->localStream[i];

		if (pLocal->eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
		{
			/** Audio payload */
			for (j = 0; j < S->remoteSession.uNumOfMedia; j++)
			{
				if (S->remoteSession.stream[j].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pRemote = &S->remoteSession.stream[j];
				}
			}

			if (pRemote == NULL)
			{
				/** Remote audio SDP is NULL... */
				uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tRemote SDP has no audio media information. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			ec_SDP_FindMatchAudioPayloads(S,
				pLocal,
				pRemote,
				&LocalAudioIndex,
				&RemoteAudioIndex,
				&LocalDTMFIndex,
				&RemoteDTMFIndex);

			/** No codecs are matched */
			if (LocalAudioIndex == -1)
			{
				uSDPError = ECRIO_SDP_CODEC_NOT_MATCHED_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tAudio codecs are not matched. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Copy matched audio media information to pInfo (overwrite) */
			EcrioSDPPayloadStruct temp[2] = {0};
			u_int8 uCnt = 0;

			temp[0].uType = pInfo->localStream[i].payload[LocalAudioIndex].uType;
			temp[0].eCodec = pInfo->localStream[i].payload[LocalAudioIndex].eCodec;
			ec_SDP_CopyAudioPayload(&(temp[0].u.audio), &(pInfo->localStream[i].payload[LocalAudioIndex].u.audio));

			if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&temp[0].u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
					__FUNCTION__, __LINE__);
				uSDPError = ECRIO_SDP_MEMORY_ERROR;
				goto END;
			}
			if ((temp[0].eCodec == EcrioAudioCodec_AMR && (param.uModeSet == 0 || param.uModeSet == 0xFF)) ||
				(temp[0].eCodec == EcrioAudioCodec_AMR_WB && (param.uModeSet == 0 || param.uModeSet == 0x1FF)))
			{
				EcrioAudioCodecAMRStruct tempParam = { 0 };

				/** Local mode-set is unrestricted, overwrite mode-set by remote parameters */
				if (KPALErrorNone != pal_MemoryCopy((void *)&tempParam, sizeof(EcrioAudioCodecAMRStruct), (void *)&pRemote->payload[RemoteAudioIndex].u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
						__FUNCTION__, __LINE__);
					uSDPError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				param.uModeSet = tempParam.uModeSet;
				if (KPALErrorNone != pal_MemoryCopy((void *)&temp[0].u.audio.uSettings, sizeof(u_int32), (void *)&param, sizeof(EcrioAudioCodecAMRStruct)))
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
						__FUNCTION__, __LINE__);
					uSDPError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				temp[0].u.audio.uOpaque |= ECRIO_SDP_SHOWN_AMR_PARAMETER_MODESET;
			}

			uCnt++;

			if (LocalDTMFIndex != -1)
			{
				temp[1].uType = pInfo->localStream[i].payload[LocalDTMFIndex].uType;
				temp[1].eCodec = pInfo->localStream[i].payload[LocalDTMFIndex].eCodec;
				ec_SDP_CopyAudioPayload(&(temp[1].u.audio), &(pInfo->localStream[i].payload[LocalDTMFIndex].u.audio));
				uCnt++;
			}

			pal_MemorySet(pInfo->localStream[i].payload, 0, sizeof(EcrioSDPPayloadStruct) * MAX_PAYLOAD);

			pInfo->localStream[i].uNumOfPayloads = uCnt;
			if (KPALErrorNone != pal_MemoryCopy(pInfo->localStream[i].payload,
					sizeof(EcrioSDPPayloadStruct) * MAX_PAYLOAD,
					&temp,
					sizeof(EcrioSDPPayloadStruct) * pInfo->localStream[i].uNumOfPayloads))
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
					__FUNCTION__, __LINE__);
				uSDPError = ECRIO_SDP_MEMORY_ERROR;
				goto END;
			}

			/** If true, then copy matched audio media information at remote end to pInfo */
			if (bAddRemote == Enum_TRUE)
			{
				pInfo->uNumOfRemoteMedia = S->remoteSession.uNumOfMedia;
				uSDPError = ec_SDP_CopyStreamStructure( S,
					&pInfo->strings,
					&pInfo->remoteStream[i],
					&S->remoteSession.stream[i]);
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
						__FUNCTION__, __LINE__, i, uSDPError);
					goto END;
				}

				uCnt = 0;
				pal_MemorySet(&temp, 0, sizeof(EcrioSDPPayloadStruct) * 2);

				if (RemoteAudioIndex != -1)
				{
					temp[0].uType = pRemote->payload[RemoteAudioIndex].uType;
					temp[0].eCodec = pRemote->payload[RemoteAudioIndex].eCodec;
					ec_SDP_CopyAudioPayload(&(temp[0].u.audio), &(pRemote->payload[RemoteAudioIndex].u.audio));
					uCnt++;
				}

				if (RemoteDTMFIndex != -1)
				{
					temp[1].uType = pRemote->payload[RemoteDTMFIndex].uType;
					temp[1].eCodec = pRemote->payload[RemoteDTMFIndex].eCodec;
					ec_SDP_CopyAudioPayload(&(temp[1].u.audio), &(pRemote->payload[RemoteDTMFIndex].u.audio));
					uCnt++;
				}

				pal_MemorySet(pInfo->remoteStream[i].payload, 0, sizeof(EcrioSDPPayloadStruct) * MAX_PAYLOAD);

				pInfo->remoteStream[i].uNumOfPayloads = uCnt;
				if (KPALErrorNone != pal_MemoryCopy(pInfo->remoteStream[i].payload,
						sizeof(EcrioSDPPayloadStruct) * MAX_PAYLOAD,
						&temp,
						sizeof(EcrioSDPPayloadStruct) * pInfo->remoteStream[i].uNumOfPayloads))
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
						__FUNCTION__, __LINE__);
					uSDPError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
			}
		}
		else if (pInfo->localStream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
		{
			/** MSRP has no payload, then no need to check media parameter */
		}
		else
		{
			uSDPError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tInvalid media type: i = %d, media type = %d,  uSDPError = %u",
				__FUNCTION__, __LINE__, i, pInfo->localStream[i].eMediaType, uSDPError);
			goto END;
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

void ec_SDP_GetPreconditionState
(
	EcrioSDPInternalSessionStruct *S,
	EcrioSDPStreamStruct *pLocalStream,
	EcrioSDPStreamStruct *pRemoteStream,
	EcrioSDPPreconditionStateEnum *pPrecondState
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPPreconditionStateEnum eState = ECRIO_SDP_PRECONDITION_STATE_NONE;
	BoolEnum bIsLocalMeet = Enum_FALSE;
	BoolEnum bIsRemoteMeet = Enum_FALSE;

	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	if (pLocalStream == NULL && pRemoteStream == NULL)
	{
		*pPrecondState = ECRIO_SDP_PRECONDITION_STATE_NONE;
		goto END;
	}

	/** No precondition */
	if (pLocalStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_INVALID &&
		pRemoteStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_INVALID)
	{
		*pPrecondState = ECRIO_SDP_PRECONDITION_STATE_NONE;
		goto END;
	}

	// @todo: we need to consider when either one does not use precondition...
	//    such as if we use precondition in offer but other party does not use precondition in answer.

	eState = ECRIO_SDP_PRECONDITION_STATE_INITIATE;

	/** Check if it is already negotiating that both local and remote curr indicates some direction. */
	if (pLocalStream->precond.e2e.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID ||
		pLocalStream->precond.local.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
	{
		if (pRemoteStream->precond.e2e.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID ||
			pRemoteStream->precond.local.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			eState = ECRIO_SDP_PRECONDITION_STATE_NEGOTIATING;
		}
	}

	/** Verify status */
	if (pLocalStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_E2E ||
		pRemoteStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_E2E)
	{
		/** E2E */
		EcrioSDPPreconditionDirectionEnum direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
		EcrioSDPPreconditionStrengthEnum strength = ECRIO_SDP_PRECONDITION_STRENGTH_INVALID;

		/** Both of them are not appeared yet. */
		if (eState == ECRIO_SDP_PRECONDITION_STATE_INITIATE)
		{
			goto END;
		}

		/** Desired adopts whichever is stronger */
		if (pLocalStream->precond.e2e.curr.direction >= pRemoteStream->precond.e2e.curr.direction)
		{
			direction = pLocalStream->precond.e2e.des.direction;
		}
		else
		{
			direction = pRemoteStream->precond.e2e.des.direction;
		}

		if (pLocalStream->precond.e2e.des.strength >= pRemoteStream->precond.e2e.des.strength)
		{
			strength = pLocalStream->precond.e2e.des.strength;
		}
		else
		{
			strength = pRemoteStream->precond.e2e.des.strength;
		}

		/** Desired: none */
		if (direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
		{
			if (strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pLocalStream->precond.e2e.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pRemoteStream->precond.e2e.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.e2e.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: send */
		else if (direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
		{
			if (strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if ((pLocalStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					 pLocalStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV) &&
					(pRemoteStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					 pRemoteStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV))
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.e2e.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: recv */
		else if (direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
		{
			if (strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if ((pLocalStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					 pLocalStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV) &&
					(pRemoteStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					 pRemoteStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV))
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.e2e.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: sendrecv */
		else if (direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
		{
			if (strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pLocalStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV ||
					pRemoteStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.e2e.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
		}

		/** Preconditions are met? */
		if(bIsLocalMeet == Enum_TRUE)
		{
			eState = ECRIO_SDP_PRECONDITION_STATE_MEET;
		}
	}
	else
	{
		/** Segmented */

		/** Check local direction */
		/** Desired: none */
		if (pRemoteStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
		{
			if (pRemoteStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pRemoteStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: send */
		else if (pRemoteStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
		{
			if (pRemoteStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pRemoteStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: recv */
		else if (pRemoteStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
		{
			if (pRemoteStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					pRemoteStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
				{
					bIsLocalMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: sendrecv */
		else if (pRemoteStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
		{
			if (pLocalStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				bIsLocalMeet = Enum_TRUE;
			}
		}

		/** Check remote direction */
		/** Desired: none */
		if (pLocalStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
		{
			if (pLocalStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
				{
					bIsRemoteMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pRemoteStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
				{
					bIsRemoteMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: send */
		else if (pLocalStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
		{
			if (pLocalStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
				{
					bIsRemoteMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND ||
					pRemoteStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
				{
					bIsRemoteMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: recv */
		else if (pLocalStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
		{
			if (pLocalStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				if (pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
				{
					bIsRemoteMeet = Enum_TRUE;
				}
			}
			else
			{
				if (pLocalStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_RECV ||
					pRemoteStream->precond.local.curr.direction >= ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
				{
					bIsRemoteMeet = Enum_TRUE;
				}
			}
		}
		/** Desired: sendrecv */
		else if (pLocalStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
		{
			if (pRemoteStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				bIsRemoteMeet = Enum_TRUE;
			}
		}

		/** Preconditions are met? */
		if(bIsLocalMeet == Enum_TRUE && bIsRemoteMeet == Enum_TRUE)
		{
			eState = ECRIO_SDP_PRECONDITION_STATE_MEET;
		}
	}

	*pPrecondState = eState;

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return;
}

SDPSESSIONHANDLE EcrioSDPSessionCreate
(
	SDPHANDLE handle,
	u_int32 uAllocationSize,
	void *pAllocated,
	u_int32 *pError
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPInternalSessionStruct *S = NULL;

	if (pError == NULL)
	{
		return NULL;
	}

	if (handle == NULL)
	{
		*pError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
		return NULL;
	}

	s = (EcrioSDPStruct *)handle;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Assign memory for the primary session structure for three (local, remote and working) buffer. */
	if (uAllocationSize >= (sizeof(EcrioSDPInternalSessionStruct)) + MAX_STRINGS * 3)
	{
		S = (EcrioSDPInternalSessionStruct *)pAllocated;
	}
	else
	{
		*pError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}

	if (S == NULL)
	{
		*pError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}

	SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tActually gets used memory in SDP session: %d bytes.",
		__FUNCTION__, __LINE__, sizeof(EcrioSDPInternalSessionStruct) + MAX_STRINGS * 3);
	SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tFor internal structure: %d bytes, MAX_STRINGS = "
		"%d bytes -- MAX_STRINGS * 3 bytes are required.",
		__FUNCTION__, __LINE__, sizeof(EcrioSDPInternalSessionStruct), MAX_STRINGS);

	pal_MemorySet(S, 0, sizeof(EcrioSDPInternalSessionStruct) + MAX_STRINGS * 3);
	S->s = s;

	S->localSession.strings.pData = (u_char *)S + sizeof(EcrioSDPInternalSessionStruct);
	S->localSession.strings.uContainerSize = MAX_STRINGS;
	S->localSession.strings.uSize = 0;

	S->remoteSession.strings.pData = S->localSession.strings.pData + MAX_STRINGS;
	S->remoteSession.strings.uContainerSize = MAX_STRINGS;
	S->remoteSession.strings.uSize = 0;

	S->work.strings.pData = S->remoteSession.strings.pData + MAX_STRINGS;
	S->work.strings.uContainerSize = MAX_STRINGS;
	S->work.strings.uSize = 0;

END:
	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return (SDPSESSIONHANDLE)S;
}

u_int32 EcrioSDPSessionDelete
(
	SDPSESSIONHANDLE session
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPInternalSessionStruct *S = NULL;

	if (session == NULL)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	S = (EcrioSDPInternalSessionStruct *)session;
	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Cleanup the session instance handle here, S. */

	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return ECRIO_SDP_NO_ERROR;
}

u_int32 EcrioSDPSessionGenerateSDPString
(
	SDPSESSIONHANDLE session,
	EcrioSDPInformationStruct *pInfo,
	EcrioSDPStringStruct *pSDP
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPInternalSessionStruct *S = NULL;
	EcrioSDPSessionStruct *pSDPStruct = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i = 0;
	EcrioSDPStreamStruct *pLocalStream = NULL, *pRemoteStream = NULL;

	if (session == NULL || pInfo == NULL || pSDP == NULL)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	S = (EcrioSDPInternalSessionStruct *)session;
	s = S->s;
	pSDPStruct = &S->work;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Check internal state */
	switch (S->eState)
	{
		case ECRIO_SDP_State_None:
		{
			if (pInfo->eEvent != ECRIO_SDP_EVENT_INITIAL_OFFER &&
				pInfo->eEvent != ECRIO_SDP_EVENT_UPDATE_OFFER)
			{
				uSDPError = ECRIO_SDP_WRONG_STATE_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tState conflict error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Store IP address if provide */
			if (pInfo->localIp.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
				pInfo->localIp.pAddress != NULL)
			{
				uSDPError = ec_SDP_FillLocalIpAddrInfo(S, pInfo, Enum_FALSE);
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFilling local IP address info error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}

			ec_SDP_InitializeSessionStruct(pSDPStruct);

			/** Create initial offer with all media information specified by default list */
			uSDPError = ec_SDP_FillSDPMessageBody(S, pSDPStruct, pInfo->uNumOfLocalMedia, pInfo->localStream, pInfo->eEvent);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill SDP message body error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Form SDP */
			uSDPError = EcrioSDPForm(pSDPStruct, pSDP->pData, pSDP->uContainerSize, &pSDP->uSize, Enum_TRUE);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tForming SDP error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Verify precondition */
			for (i = 0; i < S->localSession.uNumOfMedia; i++)
			{
				if (S->localSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pLocalStream = &S->localSession.stream[i];
				}
				else
				{
					continue;
				}
			}
			for (i = 0; i < S->remoteSession.uNumOfMedia; i++)
			{
				if (S->remoteSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pRemoteStream = &S->remoteSession.stream[i];
				}
				else
				{
					continue;
				}
			}

			/** Get precondition state */
			if (pRemoteStream != NULL)
			{
				ec_SDP_GetPreconditionState(S, pLocalStream, pRemoteStream, &S->ePrecondState);
			}
			else
			{
				if (pLocalStream != NULL)
				{
					if (pLocalStream->precond.type != ECRIO_SDP_PRECONDITION_TYPE_INVALID)
					{
						S->ePrecondState = ECRIO_SDP_PRECONDITION_STATE_INITIATE;
					}
				}
			}

			/** Change internal state to "initial local offer" */
			S->eState = ECRIO_SDP_State_InitialLocalOffer;
		}
		break;

		case ECRIO_SDP_State_InitialRemoteOffer:
		{
			if (pInfo->eEvent != ECRIO_SDP_EVENT_INITIAL_ANSWER &&
				pInfo->eEvent != ECRIO_SDP_EVENT_UPDATE_ANSWER)
			{
				uSDPError = ECRIO_SDP_WRONG_STATE_ERROR;
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tState conflict error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Store IP address if provide */
			if (pInfo->localIp.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
				pInfo->localIp.pAddress != NULL)
			{
				uSDPError = ec_SDP_FillLocalIpAddrInfo(S, pInfo, Enum_FALSE);
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFilling local IP address info error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}
			}

			/** Check media status */
			uSDPError = ec_SDP_CheckMediaStatus(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMedia status error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Find match payloads using provided payload structure via pInfo from upper layer */
			uSDPError = ec_SDP_FindMatchPayloads(S, pInfo, Enum_TRUE);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFind match payloads error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			ec_SDP_InitializeSessionStruct(pSDPStruct);

			/** Create initial answer with negotiated codecs */
			uSDPError = ec_SDP_FillSDPMessageBody(S, pSDPStruct, pInfo->uNumOfLocalMedia, pInfo->localStream, pInfo->eEvent);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill SDP message body error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Form SDP */
			uSDPError = EcrioSDPForm(pSDPStruct, pSDP->pData, pSDP->uContainerSize, &pSDP->uSize, Enum_FALSE);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tForming SDP error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Fill remote IP address for connection information */
			uSDPError = ec_SDP_FillRemoteIpAddrInfo(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill remote IP address information error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Verify precondition */
			for (i = 0; i < S->localSession.uNumOfMedia; i++)
			{
				if (S->localSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pLocalStream = &S->localSession.stream[i];
				}
				else
				{
					continue;
				}
			}
			for (i = 0; i < S->remoteSession.uNumOfMedia; i++)
			{
				if (S->remoteSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pRemoteStream = &S->remoteSession.stream[i];
				}
				else
				{
					continue;
				}
			}

			if (pLocalStream != NULL && pLocalStream->precond.type != ECRIO_SDP_PRECONDITION_TYPE_INVALID)
			{
				/** Get precondition state */
				ec_SDP_GetPreconditionState(S, pLocalStream, pRemoteStream, &S->ePrecondState);
			}
			else
			{
				/** SDP answer has no precondition attribute. */
				S->ePrecondState = ECRIO_SDP_PRECONDITION_STATE_NONE;
			}

			/** Fill precondition information */
			pInfo->ePrecondState = S->ePrecondState;

			/** Change state when there is no precondition or precondition state met. */
			if (S->ePrecondState == ECRIO_SDP_PRECONDITION_STATE_NONE ||
				S->ePrecondState == ECRIO_SDP_PRECONDITION_STATE_MEET)
			{
				/** Change internal state to "negotiated" */
				S->eState = ECRIO_SDP_State_Negotiated;
			}
			/** For precondition */
			else
			{
				/** Change internal state to initial, then re-run initiate processing. */
				S->eState = ECRIO_SDP_State_None;
			}
		}
		break;

		case ECRIO_SDP_State_Negotiated:
		case ECRIO_SDP_State_SubsequentRemoteOffer:
		{
			/** Update the local SDP information structure */
			uSDPError = ec_SDP_UpdateLocalSDPStructure(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFailed to update the SDP information. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			ec_SDP_InitializeSessionStruct(pSDPStruct);

			/** Create subsequent offer/answer with negotiated codecs */
			uSDPError = ec_SDP_FillSDPMessageBody(S, pSDPStruct, S->localSession.uNumOfMedia, S->localSession.stream, pInfo->eEvent);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill SDP message body error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Form SDP */
			uSDPError = EcrioSDPForm(pSDPStruct, pSDP->pData, pSDP->uContainerSize, &pSDP->uSize, Enum_FALSE);	// To use negotiated SDP, then bIsOffer should be false.
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tForming SDP error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Fill negotiated media information */
			pInfo->strings.uSize = 0;
			uSDPError = ec_SDP_FillNegotiatedMediaInfo(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Create sebsequent offer */
			if (S->eState == ECRIO_SDP_State_Negotiated)
			{
				/** Change internal state to "subsequent local offer" */
				S->eState = ECRIO_SDP_State_SubsequentLocalOffer;
			}
			/** Create subsequent answer */
			else
			{
				/** Fill remote IP address for connection information */
				uSDPError = ec_SDP_FillRemoteIpAddrInfo(S, pInfo);
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill remote IP address information error. uSDPError = %u",
						__FUNCTION__, __LINE__, uSDPError);
					goto END;
				}

				/** Change internal state to "negotiated" */
				S->eState = ECRIO_SDP_State_Negotiated;
			}
		}
		break;

		case ECRIO_SDP_State_InitialLocalOffer:
		case ECRIO_SDP_State_SubsequentLocalOffer:
		default:
		{
			/** State error - current state is "initial/subsequent local offer" */
			uSDPError = ECRIO_SDP_WRONG_STATE_ERROR;
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tState conflict error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tGenerated SDP length: %d bytes.",
		__FUNCTION__, __LINE__, pSDP->uSize);
	SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tString length of pInfo: %d bytes.",
		__FUNCTION__, __LINE__, pInfo->strings.uSize);
	SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tString length of local session: %d bytes, remote session: %d bytes, work: %d bytes.",
		__FUNCTION__, __LINE__, S->localSession.strings.uSize, S->remoteSession.strings.uSize, S->work.strings.uSize);

	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

u_int32 EcrioSDPSessionHandleSDPString
(
	SDPSESSIONHANDLE session,
	EcrioSDPInformationStruct *pInfo,
	EcrioSDPStringStruct *pSDP
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPInternalSessionStruct *S = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;
	u_int32 i;
	u_int32 audioindex = 0xFFFFFFFF;
	EcrioSDPStreamStruct *pLocalStream = NULL, *pRemoteStream = NULL;

	if (session == NULL || pInfo == NULL || pSDP == NULL)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	S = (EcrioSDPInternalSessionStruct *)session;
	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	/** Check internal state */
	switch (S->eState)
	{
		case ECRIO_SDP_State_None:
		{
			/** Handle initial offer */

			/** Parse SDP */
			uSDPError = EcrioSDPParse(&S->remoteSession, pSDP->pData, pSDP->uSize);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tParsing SDP error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Verify precondition */
			for (i = 0; i < S->localSession.uNumOfMedia; i++)
			{
				if (S->localSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pLocalStream = &S->localSession.stream[i];
				}
				else
				{
					continue;
				}
			}
			for (i = 0; i < S->remoteSession.uNumOfMedia; i++)
			{
				if (S->remoteSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pRemoteStream = &S->remoteSession.stream[i];
				}
				else
				{
					continue;
				}
			}

			/** Fill precondition type for remote. */
			if (pRemoteStream != NULL)
			{
				if (pRemoteStream->precond.e2e.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
				{
					pRemoteStream->precond.type = ECRIO_SDP_PRECONDITION_TYPE_E2E;
				}
				else if (pRemoteStream->precond.local.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID ||
						 pRemoteStream->precond.remote.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
				{
					pRemoteStream->precond.type = ECRIO_SDP_PRECONDITION_TYPE_SEGMENTED;
				}
				else
				{
					pRemoteStream->precond.type = ECRIO_SDP_PRECONDITION_TYPE_INVALID;
				}
			}

			/** Get precondition state */
			if (pLocalStream != NULL)
			{
				ec_SDP_GetPreconditionState(S, pLocalStream, pRemoteStream, &S->ePrecondState);
			}
			else if (pRemoteStream != NULL)
			{
				if (pRemoteStream->precond.type != ECRIO_SDP_PRECONDITION_TYPE_INVALID)
				{
					S->ePrecondState = ECRIO_SDP_PRECONDITION_STATE_INITIATE;
				}
			}

			/** Fill precondition information */
			pInfo->ePrecondState = S->ePrecondState;

			/** Fill event type to initial offer */
			pInfo->eEvent = ECRIO_SDP_EVENT_INITIAL_OFFER;

			/** Fill IP address */
			uSDPError = ec_SDP_FillRemoteIpAddrInfo(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill remote IP address information error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Fill all media information */
			pInfo->uNumOfRemoteMedia = S->remoteSession.uNumOfMedia;

			for (i = 0; i < S->remoteSession.uNumOfMedia; ++i)
			{
				uSDPError = ec_SDP_CopyStreamStructure( S,
					&pInfo->strings,
					&pInfo->remoteStream[i],
					&S->remoteSession.stream[i]);
				if (uSDPError != ECRIO_SDP_NO_ERROR)
				{
					SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
						__FUNCTION__, __LINE__, i, uSDPError);
					goto END;
				}
			}

			/** Overwrite event type to update offer if has received precondition before */
			if (S->ePrecondState >= ECRIO_SDP_PRECONDITION_STATE_NEGOTIATING)
			{
				pInfo->eEvent = ECRIO_SDP_EVENT_UPDATE_OFFER;

				for (i = 0; i < pInfo->uNumOfLocalMedia; i++)
				{
					if (pInfo->localStream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
					{
						/** If precond state >= negotiating, then store remote precondition status to local structure */
						pInfo->localStream[i].precond.remote.curr.direction = pInfo->remoteStream[i].precond.local.curr.direction;
					}
				}
			}

			/** Change internal state to "initial remote offer" */
			S->eState = ECRIO_SDP_State_InitialRemoteOffer;
		}
		break;

		case ECRIO_SDP_State_InitialLocalOffer:
		{
			/** Handle initial answer */

			/** Parse SDP */
			uSDPError = EcrioSDPParse(&S->remoteSession, pSDP->pData, pSDP->uSize);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tParsing SDP error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Fill event type to initial answer */
			pInfo->eEvent = ECRIO_SDP_EVENT_INITIAL_ANSWER;

			/** Fill IP address */
			uSDPError = ec_SDP_FillRemoteIpAddrInfo(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill remote IP address information error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Fill negotiated media information */
			pInfo->uNumOfLocalMedia = S->localSession.uNumOfMedia;
			pInfo->uNumOfRemoteMedia = S->remoteSession.uNumOfMedia;

			/** Verify precondition */
			for (i = 0; i < S->localSession.uNumOfMedia; i++)
			{
				if (S->localSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pLocalStream = &S->localSession.stream[i];
				}
				else
				{
					continue;
				}
			}
			for (i = 0; i < S->remoteSession.uNumOfMedia; i++)
			{
				if (S->remoteSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
				{
					pRemoteStream = &S->remoteSession.stream[i];
				}
				else
				{
					continue;
				}
			}

			ec_SDP_GetPreconditionState(S, pLocalStream, pRemoteStream, &S->ePrecondState);

			/** Fill precondition information */
			pInfo->ePrecondState = S->ePrecondState;

			/** Fill precondition type for remote. */
			if (pRemoteStream != NULL)
			{
				if (pRemoteStream->precond.e2e.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
				{
					pRemoteStream->precond.type = ECRIO_SDP_PRECONDITION_TYPE_E2E;
				}
				else if (pRemoteStream->precond.local.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID ||
					pRemoteStream->precond.remote.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
				{
					pRemoteStream->precond.type = ECRIO_SDP_PRECONDITION_TYPE_SEGMENTED;
				}
				else
				{
					/** SDP answer has no precondition attribute, this is including case of which SDP offer
					 *  that we sent is including precondition but remote unwanted to use precondition.
					 */
					pRemoteStream->precond.type = ECRIO_SDP_PRECONDITION_TYPE_INVALID;
					S->ePrecondState = ECRIO_SDP_PRECONDITION_STATE_NONE;
					pInfo->ePrecondState = S->ePrecondState;
				}
			}

			/** Verify media and change state when there is no precondition or precondition state met. */
			if (S->ePrecondState == ECRIO_SDP_PRECONDITION_STATE_NONE ||
				S->ePrecondState == ECRIO_SDP_PRECONDITION_STATE_MEET)
			{
				for (i = 0; i < S->remoteSession.uNumOfMedia; ++i)
				{
					if (S->remoteSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
					{
						audioindex = i;

						/** When media type is audio, we need to update the local SDP that has only negotiated
						  * payload information for handling negotiated SDP on the next */
						uSDPError = ec_SDP_CopyStreamStructure( S,
							&pInfo->strings,
							&pInfo->localStream[i],
							&S->localSession.stream[i]);
						if (uSDPError != ECRIO_SDP_NO_ERROR)
						{
							SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
								__FUNCTION__, __LINE__, i, uSDPError);
							goto END;
						}

						uSDPError = ec_SDP_FindMatchPayloads(S, pInfo, Enum_TRUE);
						if (uSDPError != ECRIO_SDP_NO_ERROR)
						{
							SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFind match payloads error. uSDPError = %u",
								__FUNCTION__, __LINE__, uSDPError);
							goto END;
						}

						S->localSession.stream[i].uNumOfPayloads = pInfo->localStream[i].uNumOfPayloads;
						if (KPALErrorNone != pal_MemoryCopy(S->localSession.stream[i].payload,
								sizeof(EcrioSDPPayloadStruct) * MAX_PAYLOAD,
								pInfo->localStream[i].payload,
								sizeof(EcrioSDPPayloadStruct) * MAX_PAYLOAD))
						{
							SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
								__FUNCTION__, __LINE__);
							uSDPError = ECRIO_SDP_MEMORY_ERROR;
							goto END;
						}

						/** Reset strings structure */
						pInfo->strings.uSize = 0;
					}
					else if (S->remoteSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
					{
						/** Fill remote media information */
						uSDPError = ec_SDP_CopyStreamStructure(S,
							&pInfo->strings,
							&pInfo->remoteStream[i],
							&S->remoteSession.stream[i]);
						if (uSDPError != ECRIO_SDP_NO_ERROR)
						{
							SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
								__FUNCTION__, __LINE__, i, uSDPError);
							goto END;
						}
					}
				}

				/** Change internal state to "negotiated" */
				S->eState = ECRIO_SDP_State_Negotiated;

				if (S->ePrecondState == ECRIO_SDP_PRECONDITION_STATE_MEET)
				{
					/** Overwrite event type to update answer */
					pInfo->eEvent = ECRIO_SDP_EVENT_UPDATE_ANSWER;

					/** Update current status of remote in local stream for later */
					if (audioindex != 0xFFFFFFFF)
					{
						S->localSession.stream[audioindex].precond.remote.curr.direction = S->remoteSession.stream[audioindex].precond.local.curr.direction;
						pInfo->localStream[audioindex].precond.remote.curr.direction = pInfo->remoteStream[audioindex].precond.local.curr.direction;
					}
				}
			}
			/** For precondition */
			else
			{
				for (i = 0; i < S->remoteSession.uNumOfMedia; ++i)
				{
					if (S->remoteSession.stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
					{
						/** Fill local and remote media information */
						uSDPError = ec_SDP_CopyStreamStructure(S,
							&pInfo->strings,
							&pInfo->localStream[i],
							&S->localSession.stream[i]);
						if (uSDPError != ECRIO_SDP_NO_ERROR)
						{
							SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
								__FUNCTION__, __LINE__, i, uSDPError);
							goto END;
						}

						uSDPError = ec_SDP_CopyStreamStructure(S,
							&pInfo->strings,
							&pInfo->remoteStream[i],
							&S->remoteSession.stream[i]);
						if (uSDPError != ECRIO_SDP_NO_ERROR)
						{
							SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. i = %d, uSDPError = %u",
								__FUNCTION__, __LINE__, i, uSDPError);
							goto END;
						}
					}
				}

				/** Change internal state to initial, then re-run initiate processing. */
				S->eState = ECRIO_SDP_State_None;
			}
		}
		break;

		case ECRIO_SDP_State_Negotiated:
		case ECRIO_SDP_State_SubsequentLocalOffer:
		{
			/** Copy remote SDP to the working buffer */
			uSDPError = ec_SDP_CopySessionStructure(S,
				&S->work,
				&S->remoteSession);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tSession information copy error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Reset strings buffer */
			S->remoteSession.strings.uSize = 0;

			/** Parse SDP */
			uSDPError = EcrioSDPParse(&S->remoteSession, pSDP->pData, pSDP->uSize);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tParsing SDP error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Check what parameter has been changed based on the previous remote sdp */
			uSDPError = ec_SDP_CheckSDPEvent(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tChecking SDP event error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Fill IP address */
			pInfo->strings.uSize = 0;

			uSDPError = ec_SDP_FillRemoteIpAddrInfo(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tFill remote IP address information error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Fill negotiated media information */
			uSDPError = ec_SDP_FillNegotiatedMediaInfo(S, pInfo);
			if (uSDPError != ECRIO_SDP_NO_ERROR)
			{
				SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tStream information copy error. uSDPError = %u",
					__FUNCTION__, __LINE__, uSDPError);
				goto END;
			}

			/** Handle subsequent offer */
			if (S->eState == ECRIO_SDP_State_Negotiated)
			{
				/** Change internal state to "subsequent remote offer" */
				S->eState = ECRIO_SDP_State_SubsequentRemoteOffer;
			}
			/** Handle subsequent answer */
			else
			{
				/** Change internal state to "negotiated" */
				S->eState = ECRIO_SDP_State_Negotiated;
			}
		}
		break;

		case ECRIO_SDP_State_InitialRemoteOffer:
		case ECRIO_SDP_State_SubsequentRemoteOffer:
		default:
		{
			/** State error - current state is "initial/subsequent remote offer" */
			uSDPError = ECRIO_SDP_WRONG_STATE_ERROR;
			SDPLOGE(s->logHandle, KLogTypeGeneral, "%s:%u\tState conflict error. uSDPError = %u",
				__FUNCTION__, __LINE__, uSDPError);
			goto END;
		}
	}

END:
	SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tString length of pInfo: %d bytes.",
		__FUNCTION__, __LINE__, pInfo->strings.uSize);
	SDPLOGI(s->logHandle, KLogTypeGeneral, "%s:%u\tString length of local session: %d bytes, remote session: %d bytes, work: %d bytes.",
		__FUNCTION__, __LINE__, S->localSession.strings.uSize, S->remoteSession.strings.uSize, S->work.strings.uSize);

	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}

u_int32 EcrioSDPFindMatchPayloads
(
	SDPSESSIONHANDLE session,
	EcrioSDPInformationStruct *pInfo
)
{
	EcrioSDPStruct *s = NULL;
	EcrioSDPInternalSessionStruct *S = NULL;
	u_int32 uSDPError = ECRIO_SDP_NO_ERROR;

	if (session == NULL || pInfo == NULL)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	S = (EcrioSDPInternalSessionStruct *)session;
	s = S->s;

	SDPLOGI(s->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	uSDPError = ec_SDP_FindMatchPayloads(S, pInfo, Enum_TRUE);

	SDPLOGI(s->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);
	return uSDPError;
}
