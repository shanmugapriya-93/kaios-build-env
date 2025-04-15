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
* @file EcrioSDPForm.c
* @brief Implementation of SDP module forming functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

u_int32 ec_SDP_FormVLine
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;

	pSdpStruct = pSdpStruct;

	/** Create "v=" line */
	uLength = 5;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"v=0", 3, uError);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt + 3, ECRIO_SDP_CRLF_SYMBOL, 2, uError);
	uCnt += uLength;

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormOLine
(
	EcrioSDPOriginStruct *pOrigin,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;
	u_char buf[32] = {0};

	/** Create "o=" line */
	uLength = 2;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"o=", 2, uError);
	uCnt += uLength;

	/** username */
#if 0
	uLength = pal_StringLength(pOrigin->pUsername);
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pOrigin->pUsername, uLength, uError);
	uCnt += uLength;
#else
	uLength = pal_StringLength((u_char*)"-");
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char*)"-", uLength, uError);
	uCnt += uLength;
#endif

	/** sess-id */
	pal_MemorySet(buf, 0, 32);
	if (0 >= pal_StringSNPrintf((char *)buf, 32, " %llu", pOrigin->uSessionid))
	{
		uError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}
	uLength = pal_StringLength(buf);
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char*)buf, uLength, uError);
	uCnt += uLength;

	/** sess-version */
	pal_MemorySet(buf, 0, 32);
	if (0 >= pal_StringSNPrintf((char *)buf, 32, " %llu", pOrigin->uVersion))
	{
		uError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}
	uLength = pal_StringLength(buf);
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char*)buf, uLength, uError);
	uCnt += uLength;

	/** nettype */
	uLength = 4;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char*)" IN ", uLength, uError);
	uCnt += uLength;

	/** addrtype */
	if (pOrigin->eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4)
	{
		uLength = 4;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char*)"IP4 ", uLength, uError);
		uCnt += uLength;
	}
	else if (pOrigin->eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6)
	{
		uLength = 4;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char*)"IP6 ", uLength, uError);
		uCnt += uLength;
	}
	else
	{
		uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
		goto END;
	}



	/** unicast-address */
	/* if IPV6*/
	if (pOrigin->eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6)
	{
		uLength = pal_StringLength((u_char *)"[");
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char*)"[", uLength, uError);
		uCnt += uLength;
	}

	uLength = pal_StringLength(pOrigin->pAddress);
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pOrigin->pAddress, uLength, uError);
	uCnt += uLength;

	/* if IPV6*/
	if (pOrigin->eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6)
	{
		uLength = pal_StringLength((u_char *)"]");
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"]", uLength, uError);
		uCnt += uLength;
	}


	/** CRLF */
	uLength = 2;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
	uCnt += uLength;

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormSLine
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;

	pSdpStruct = pSdpStruct;

	/** Create "s=" line */
	/** Currently session name field does not set a specify name. */
	uLength = 5;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"s=-", 3, uError);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt + 3, ECRIO_SDP_CRLF_SYMBOL, 2, uError);
	uCnt += uLength;

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormCLine
(
	EcrioSDPConnectionInfomationStruct *pConn,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;

	/** Create "c=" line */
	if (pConn->eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE)
	{
		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"c=", uLength, uError);
		uCnt += uLength;

		/** nettype */
		uLength = 3;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"IN ", uLength, uError);
		uCnt += uLength;

		/** addrtype */
		if (pConn->eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP4)
		{
			uLength = 4;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"IP4 ", uLength, uError);
			uCnt += uLength;
		}
		else if (pConn->eAddressType == ECRIO_SDP_ADDRESS_TYPE_IP6)
		{
			uLength = 4;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"IP6 ", uLength, uError);
			uCnt += uLength;
		}
		else
		{
			uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
			goto END;
		}
	}

	/** unicast-address */
	uLength = pal_StringLength(pConn->pAddress);
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pConn->pAddress, uLength, uError);
	uCnt += uLength;

	/** CRLF */
	uLength = 2;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
	uCnt += uLength;

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormBLine
(
	EcrioSDPBandwidthStruct *pBandwidth,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;
	u_char buf[32] = {0};

	/** Create "b=" line */

	/** AS */
	if ((pBandwidth->uModifier & ECRIO_SDP_BANDWIDTH_MODIFIER_AS) != 0)
	{
		uLength = 5;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"b=AS:", uLength, uError);
		uCnt += uLength;

		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pBandwidth->uAS))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** RS */
	if ((pBandwidth->uModifier & ECRIO_SDP_BANDWIDTH_MODIFIER_RS) != 0)
	{
		uLength = 5;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"b=RS:", uLength, uError);
		uCnt += uLength;

		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pBandwidth->uRS))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** RR */
	if ((pBandwidth->uModifier & ECRIO_SDP_BANDWIDTH_MODIFIER_RR) != 0)
	{
		uLength = 5;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"b=RR:", uLength, uError);
		uCnt += uLength;

		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pBandwidth->uRR))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormTLine
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;

	pSdpStruct = pSdpStruct;

	/** Create "t=" line */

	/** Currently start and end time fields are set 0. */
	uLength = 7;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"t=0 0", 5, uError);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt + 5, ECRIO_SDP_CRLF_SYMBOL, 2, uError);
	uCnt += uLength;

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormALine_Precondition
(
	EcrioSDPStreamStruct *pStream,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;

	/** current-status */
	if (pStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_E2E)
	{
		if (pStream->precond.e2e.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=curr:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
	}
	else if (pStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_SEGMENTED)
	{
		/** local */
		if (pStream->precond.local.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=curr:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.local.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}

		/** remote */
		if (pStream->precond.remote.curr.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=curr:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.remote.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.curr.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
	}

	/** desired-status */
	if (pStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_E2E)
	{
		if (pStream->precond.e2e.des.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 6;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=des:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** strength-type */
			if (pStream->precond.e2e.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING, uLength, uError);
			}
			else
			{
				uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				goto END;
			}
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.e2e.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
	}
	else if (pStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_SEGMENTED)
	{
		/** local */
		if (pStream->precond.local.des.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 6;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=des:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** strength-type */
			if (pStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING, uLength, uError);
			}
			else if (pStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING, uLength, uError);
			}
			else if (pStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING, uLength, uError);
			}
			else if (pStream->precond.local.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING, uLength, uError);
			}
			else
			{
				uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				goto END;
			}
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.local.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}

		/** remote */
		if (pStream->precond.remote.des.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 6;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=des:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** strength-type */
			if (pStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.des.strength == ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING, uLength, uError);
			}
			else
			{
				uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				goto END;
			}
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.des.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
	}

	/** confirm-status */
	if (pStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_E2E)
	{
		if (pStream->precond.e2e.conf.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=conf:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.e2e.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.e2e.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
	}
	else if (pStream->precond.type == ECRIO_SDP_PRECONDITION_TYPE_SEGMENTED)
	{
		/** local */
		if (pStream->precond.local.conf.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=conf:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.local.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.local.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.local.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.local.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}

		/** remote */
		if (pStream->precond.remote.conf.direction != ECRIO_SDP_PRECONDITION_DIRECTION_INVALID)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=conf:", uLength, uError);
			uCnt += uLength;

			/** precondition-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** status-type */
			uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			/** direction-tag */
			if (pStream->precond.remote.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_NONE)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SEND)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_RECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING, uLength, uError);
			}
			else if (pStream->precond.remote.conf.direction == ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV)
			{
				uLength = pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING, uLength, uError);
			}
			uCnt += uLength;

			uLength = 2;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
	}

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormALine_ModeAttribute
(
	EcrioSDPStreamStruct *pStream,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;

	/** sendonly */
	if (pStream->eDirection == ECRIO_SDP_MEDIA_DIRECTION_SENDONLY)
	{
		uLength = 10;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=sendonly", uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}
	/** recvonly */
	else if (pStream->eDirection == ECRIO_SDP_MEDIA_DIRECTION_RECVONLY)
	{
		uLength = 10;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=recvonly", uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}
	/** inactive */
	else if (pStream->eDirection == ECRIO_SDP_MEDIA_DIRECTION_INACTIVE)
	{
		uLength = 10;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=inactive", uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}
	/** sendrecv */
	else if (pStream->eDirection == ECRIO_SDP_MEDIA_DIRECTION_SENDRECV)
	{
		uLength = 10;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=sendrecv", uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormALine_EncodeClockChannel
(
	EcrioSDPStreamStruct *pStream,
	u_int32 i,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uLength = 0;
	u_int32	uCnt = *pCnt;
	u_char buf[32] = {0};

	switch (pStream->payload[i].eCodec)
	{
		case EcrioAudioCodec_PCMU:
		{
			uLength = 4;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"PCMU", uLength, uError);
			break;
		}

		case EcrioAudioCodec_L16:
		{
			uLength = 3;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"L16", uLength, uError);
		}
		break;

		case EcrioAudioCodec_AMR:
		{
			uLength = 3;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"AMR", uLength, uError);
		}
		break;

		case EcrioAudioCodec_AMR_WB:
		{
			uLength = 6;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"AMR-WB", uLength, uError);
		}
		break;

		case EcrioAudioCodec_DTMF:
		{
			uLength = 15;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"telephone-event", uLength, uError);
		}
		break;

		default:
		{
		}
	}
	uCnt += uLength;

	uLength = 1;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SLASH_SYMBOL, uLength, uError);
	uCnt += uLength;

	pal_MemorySet(buf, 0, 32);
	if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->payload[i].u.audio.uClockrate))
	{
		uError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}
	uLength = pal_StringLength(buf);
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
	uCnt += uLength;

	if (pStream->payload[i].u.audio.uChannels > 0)
	{
		uLength = 1;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SLASH_SYMBOL, uLength, uError);
		uCnt += uLength;

		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->payload[i].u.audio.uChannels))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;
	}

	uLength = 2;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
	uCnt += uLength;

	*pCnt = uCnt;
END:
	return uError;
}

u_int32 ec_SDP_FormMLine_Audio
(
	EcrioSDPStreamStruct *pStream,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt,
	BoolEnum bIsOffer
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	i;
	u_int32	uCnt;
	u_int32	uLength = 0;
	u_char buf[32] = {0};

	uCnt = *pCnt;

	/** Create Media ("m=") line for audio */
	{
		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"m=", uLength, uError);
		uCnt += uLength;

		/** media */
		uLength = 6;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"audio ", uLength, uError);
		uCnt += uLength;

		/** port */
		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d ", pStream->uMediaPort))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		/** protocol */
		if (pStream->eProtocol == ECRIO_SDP_MEDIA_PROTOCOL_RTP_AVP)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"RTP/AVP", uLength, uError);
			uCnt += uLength;
		}
		else
		{
			uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
			goto END;
		}

		/** format */
		for (i = 0; i < pStream->uNumOfPayloads; i++)
		{
			/** Skip if this codec type is AMR/AMR-WB and this is only used to
			  * compare the offer's supported codec list in answer mode.
			  * I.e., we may not want to add this codec in offers. */
			if (pStream->payload[i].eCodec == EcrioAudioCodec_AMR ||
				pStream->payload[i].eCodec == EcrioAudioCodec_AMR_WB)
			{
				if (bIsOffer == Enum_TRUE && pStream->payload[i].u.audio.bFlag == Enum_TRUE)
				{
					continue;
				}
			}

			pal_MemorySet(buf, 0, 32);
			if (0 >= pal_StringSNPrintf((char *)buf, 32, " %d", pStream->payload[i].uType))
			{
				uError = ECRIO_SDP_MEMORY_ERROR;
				goto END;
			}
			uLength = pal_StringLength(buf);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
			uCnt += uLength;
		}

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** Create "c=" line into the media description */
	if (pStream->conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
		pStream->conn.pAddress != NULL)
	{
		uError = ec_SDP_FormCLine(&pStream->conn, pBuffer, uContainerSize, &uCnt);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	/** Create bandwidth ("b=") line into the media description */
	if (pStream->bandwidth.uModifier != ECRIO_SDP_BANDWIDTH_MODIFIER_NONE)
	{
		uError = ec_SDP_FormBLine(&pStream->bandwidth, pBuffer, uContainerSize, &uCnt);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	/** Create Attribute ("a=") line */
	/** payloads */
	for (i = 0; i < pStream->uNumOfPayloads; i++)
	{
		/** Skip if codec type not supported */
		if (pStream->payload[i].eCodec == EcrioAudioCodec_None ||
			pStream->payload[i].eCodec >= EcrioAudioCodec_Last)
		{
			continue;
		}

		/** Skip if this codec type is AMR/AMR-WB and this is only used to
		  * compare the offer's supported codec list in answer mode.
		  * I.e., we may not want to add this codec in offers. */
		if (pStream->payload[i].eCodec == EcrioAudioCodec_AMR ||
			pStream->payload[i].eCodec == EcrioAudioCodec_AMR_WB)
		{
			if (bIsOffer == Enum_TRUE && pStream->payload[i].u.audio.bFlag == Enum_TRUE)
			{
				continue;
			}
		}

		/** rtpmap */
		uLength = 9;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=rtpmap:", uLength, uError);
		uCnt += uLength;

		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d ", pStream->payload[i].uType))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		switch (pStream->payload[i].eCodec)
		{
			case EcrioAudioCodec_PCMU:
			case EcrioAudioCodec_L16:
			{
				uError = ec_SDP_FormALine_EncodeClockChannel(pStream, i, pBuffer, uContainerSize, &uCnt);
				if (uError != ECRIO_SDP_NO_ERROR)
				{
					goto END;
				}
			}
			break;

#if !defined(DISABLE_CODEC_AMR_NB) || !defined(DISABLE_CODEC_AMR_WB)
			case EcrioAudioCodec_AMR:
			case EcrioAudioCodec_AMR_WB:
			{
				u_int16 uVal;
				EcrioAudioCodecAMRStruct param = { 0 };

				uError = ec_SDP_FormALine_EncodeClockChannel(pStream, i, pBuffer, uContainerSize, &uCnt);
				if (uError != ECRIO_SDP_NO_ERROR)
				{
					goto END;
				}

				/** AMR parameters */
				/** fmtp */
				if (pStream->payload[i].u.audio.uOpaque != ECRIO_SDP_SHOWN_AMR_PARAMETER_NONE)
				{
					if (KPALErrorNone != pal_MemoryCopy((void *)&param, sizeof(EcrioAudioCodecAMRStruct), (void *)&pStream->payload[i].u.audio.uSettings, sizeof(EcrioAudioCodecAMRStruct)))
					{
						uError = ECRIO_SDP_MEMORY_ERROR;
						goto END;
					}

					uLength = 7;
					CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
					SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=fmtp:", uLength, uError);
					uCnt += uLength;

					pal_MemorySet(buf, 0, 32);
					if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->payload[i].uType))
					{
						uError = ECRIO_SDP_MEMORY_ERROR;
						goto END;
					}
					uLength = pal_StringLength(buf);
					CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
					SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
					uCnt += uLength;

					uLength = 1;
					CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
					SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
					uCnt += uLength;

					/** octet-align */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_OCTET_ALIGN) != 0)
					{
						uLength = 12;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"octet-align=", uLength, uError);
						uCnt += uLength;

						uLength = 3;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						if (param.uOctetAlign != 0)
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"1; ", uLength, uError);
						}
						else
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"0; ", uLength, uError);
						}

						uCnt += uLength;
					}

					/** mode-set */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_MODESET) != 0)
					{
						u_int8 uBitCnt;

						uVal = (u_int16)param.uModeSet;
						if (uVal != 0)
						{
							uLength = 9;
							CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"mode-set=", uLength, uError);
							uCnt += uLength;

							for (uBitCnt = 0; uBitCnt < 9; uBitCnt++)
							{
								if (((uVal >> uBitCnt) & 0x0001) != 0)
								{
									pal_MemorySet(buf, 0, 32);
									if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", uBitCnt))
									{
										uError = ECRIO_SDP_MEMORY_ERROR;
										goto END;
									}
									uLength = pal_StringLength(buf);
									CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
									SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
									uCnt += uLength;

									uLength = 1;
									CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
									SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_COMMA_SYMBOL, uLength, uError);
									uCnt += uLength;
								}
							}

							/** Remove a last comma */
							uCnt--;
							*(pBuffer + uCnt) = 0x00;

							uLength = 2;
							CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SEMICOLON_SYMBOL, 1, uError);
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt + 1, ECRIO_SDP_SPACE_SYMBOL, 1, uError);
							uCnt += uLength;
						}
					}

					/** mode-change-period */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_PERIOD) != 0)
					{
						uLength = 19;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"mode-change-period=", uLength, uError);
						uCnt += uLength;

						uLength = 3;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						if (param.uModeChangePeriod != 0)
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"2; ", uLength, uError);
						}
						else
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"1; ", uLength, uError);
						}

						uCnt += uLength;
					}

					/** mode-change-capability */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_CAPABILITY) != 0)
					{
						uLength = 23;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"mode-change-capability=", uLength, uError);
						uCnt += uLength;

						uLength = 3;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						if (param.uModeChangeCapability != 0)
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"2; ", uLength, uError);
						}
						else
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"1; ", uLength, uError);
						}

						uCnt += uLength;
					}

					/** mode-change-neighbor */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_MODE_CHANGE_NEIGHBOR) != 0)
					{
						uLength = 21;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"mode-change-neighbor=", uLength, uError);
						uCnt += uLength;

						uLength = 3;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						if (param.uModeChangeNeighbor != 0)
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"1; ", uLength, uError);
						}
						else
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"0; ", uLength, uError);
						}

						uCnt += uLength;
					}

					/** crc */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_CRC) != 0)
					{
						uLength = 4;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"crc=", uLength, uError);
						uCnt += uLength;

						uLength = 3;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						if (param.uCRC != 0)
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"1; ", uLength, uError);
						}
						else
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"0; ", uLength, uError);
						}

						uCnt += uLength;
					}

					/** robust-sorting */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_ROBUST_SORTING) != 0)
					{
						uLength = 15;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"robust-sorting=", uLength, uError);
						uCnt += uLength;

						uLength = 3;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						if (param.uRobustSorting != 0)
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"1; ", uLength, uError);
						}
						else
						{
							SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"0; ", uLength, uError);
						}

						uCnt += uLength;
					}

					/** max-red */
					if ((pStream->payload[i].u.audio.uOpaque & ECRIO_SDP_SHOWN_AMR_PARAMETER_MAX_RED) != 0)
					{
						uVal = (u_int16)param.uMaxRed;

						uLength = 8;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"max-red=", uLength, uError);
						uCnt += uLength;

						pal_MemorySet(buf, 0, 32);
						if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", uVal))
						{
							uError = ECRIO_SDP_MEMORY_ERROR;
							goto END;
						}
						uLength = pal_StringLength(buf);
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
						uCnt += uLength;

						uLength = 2;
						CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SEMICOLON_SYMBOL, 1, uError);
						SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt + 1, ECRIO_SDP_SPACE_SYMBOL, 1, uError);
						uCnt += uLength;
					}

					/** Remove a last semicolon and whitespace */
					uCnt -= 2;
					*(pBuffer + uCnt) = 0x00;
					*(pBuffer + uCnt + 1) = 0x00;

					uLength = 2;
					CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
					SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
					uCnt += uLength;
				}
			}
			break;
#endif

			case EcrioAudioCodec_DTMF:
			{
				u_int32 uStart;
				u_int32 uStop;

				uError = ec_SDP_FormALine_EncodeClockChannel(pStream, i, pBuffer, uContainerSize, &uCnt);
				if (uError != ECRIO_SDP_NO_ERROR)
				{
					goto END;
				}

				/** fmtp */
				uLength = 7;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=fmtp:", uLength, uError);
				uCnt += uLength;

				pal_MemorySet(buf, 0, 32);
				if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->payload[i].uType))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				uLength = pal_StringLength(buf);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
				uCnt += uLength;

				uStart = pStream->payload[i].u.audio.uSettings >> 24;
				uStop = pStream->payload[i].u.audio.uSettings & 0x00FF0000;
				uStop >>= 16;

				pal_MemorySet(buf, 0, 32);
				if (0 >= pal_StringSNPrintf((char *)buf, 32, " %d-%d", (u_int8)uStart, (u_int8)uStop))
				{
					uError = ECRIO_SDP_MEMORY_ERROR;
					goto END;
				}
				uLength = pal_StringLength(buf);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
				uCnt += uLength;

				uLength = 2;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
				uCnt += uLength;
			}
			break;

			default:
			{
			}
			break;
		}
	}

	/** Precondition */
	uError = ec_SDP_FormALine_Precondition(pStream, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	/** Mode attribute */
	uError = ec_SDP_FormALine_ModeAttribute(pStream, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	/** ptime */
	if (pStream->u.audio.uPtime > 0)
	{
		uLength = 8;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=ptime:", uLength, uError);
		uCnt += uLength;

		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->u.audio.uPtime))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** maxptime */
	if (pStream->u.audio.uMaxPtime > 0)
	{
		uLength = 11;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=maxptime:", uLength, uError);
		uCnt += uLength;

		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->u.audio.uMaxPtime))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

END:
	*pCnt = uCnt;
	return uError;
}

u_int32 ec_SDP_FormMLine_MSRP
(
	EcrioSDPStreamStruct *pStream,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pCnt
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	uCnt;
	u_int32	uLength = 0;
	u_char buf[32] = {0};
	BoolEnum bIsAddParam = Enum_FALSE;

	uCnt = *pCnt;

	/** Create Media ("m=") line for MSRP */
	uLength = 2;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"m=", uLength, uError);
	uCnt += uLength;

	/** media */
	uLength = 8;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"message ", uLength, uError);
	uCnt += uLength;

	/** port */
	pal_MemorySet(buf, 0, 32);
	if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d ", pStream->uMediaPort))
	{
		uError = ECRIO_SDP_MEMORY_ERROR;
		goto END;
	}
	uLength = pal_StringLength(buf);
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
	uCnt += uLength;

	/** protocol */
	if (pStream->eProtocol == ECRIO_SDP_MEDIA_PROTOCOL_TCP_MSRP)
	{
		uLength = 9;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"TCP/MSRP ", uLength, uError);
		uCnt += uLength;
	}
	else if (pStream->eProtocol == ECRIO_SDP_MEDIA_PROTOCOL_TLS_MSRP)
	{
		uLength = 13;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"TCP/TLS/MSRP ", uLength, uError);
		uCnt += uLength;
	}
	else
	{
		uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
		goto END;
	}

	/** format - set "*" */
	uLength = 1;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_ASTERISK_SYMBOL, uLength, uError);
	uCnt += uLength;

	/** CRLF */
	uLength = 2;
	CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
	SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
	uCnt += uLength;

	/** Create "c=" line into the media description */
	if (pStream->conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
		pStream->conn.pAddress != NULL)
	{
		uError = ec_SDP_FormCLine(&pStream->conn, pBuffer, uContainerSize, &uCnt);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	/** Create bandwidth ("b=") line into the media description */
	if (pStream->bandwidth.uModifier != ECRIO_SDP_BANDWIDTH_MODIFIER_NONE)
	{
		uError = ec_SDP_FormBLine(&pStream->bandwidth, pBuffer, uContainerSize, &uCnt);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	/** Create Attribute ("a=") line */
	/** accept-types */
	if (pStream->u.msrp.eAcceptTypes != ECRIO_SDP_MSRP_MEDIA_TYPE_NONE)
	{
		uLength = 15;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=accept-types:", uLength, uError);
		uCnt += uLength;

		if (pStream->u.msrp.eAcceptTypes == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD)
		{
			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_ASTERISK_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
		else if (pStream->u.msrp.eAcceptTypes == ECRIO_SDP_MSRP_MEDIA_TYPE_GENERAL)
		{
			if (pStream->u.msrp.fileSelector.pType == NULL)
			{
				uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
				goto END;
			}
			uLength = pal_StringLength(pStream->u.msrp.fileSelector.pType);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.fileSelector.pType, uLength, uError);
			uCnt += uLength;
		}
		else
		{
			bIsAddParam = Enum_FALSE;
			if ((pStream->u.msrp.eAcceptTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN) != 0)
			{
				uLength = 11;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"text/plain ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM) != 0)
			{
				uLength = 13;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"message/cpim ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML) != 0)
			{
				uLength = 17;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"message/imdn+xml ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML) != 0)
			{
				uLength = 31;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"application/im-iscomposing+xml ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA) != 0)
			{
				uLength = 38;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"application/vnd.oma.cpm-groupdata+xml ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if (bIsAddParam == Enum_TRUE)
			{
				/** Remove a last space */
				uCnt--;
				*(pBuffer + uCnt) = 0x00;
			}
		}

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** accept-wrapped-types */
	if (pStream->u.msrp.eAcceptWrappedTypes != ECRIO_SDP_MSRP_MEDIA_TYPE_NONE)
	{
		uLength = 23;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=accept-wrapped-types:", uLength, uError);
		uCnt += uLength;

		if (pStream->u.msrp.eAcceptWrappedTypes == ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD)
		{
			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_ASTERISK_SYMBOL, uLength, uError);
			uCnt += uLength;
		}
		else
		{
			bIsAddParam = Enum_FALSE;
			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN) != 0)
			{
				uLength = 11;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"text/plain ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE) != 0)
			{
				uLength = 32;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"application/conference-info+xml ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM) != 0)
			{
				uLength = 13;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"message/cpim ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML) != 0)
			{
				uLength = 17;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"message/imdn+xml ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML) != 0)
			{
				uLength = 31;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"application/im-iscomposing+xml ", uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA) != 0)
			{
				uLength = pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA_STRING);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA_STRING, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}

			if ((pStream->u.msrp.eAcceptWrappedTypes & ECRIO_SDP_MSRP_MEDIA_TYPE_GENERAL) != 0)
			{
				if (pStream->u.msrp.fileSelector.pType == NULL)
				{
					uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
					goto END;
				}
				uLength = pal_StringLength(pStream->u.msrp.fileSelector.pType);
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.fileSelector.pType, uLength, uError);
				uCnt += uLength;
				uLength = 1;
				CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
				SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
				uCnt += uLength;
				bIsAddParam = Enum_TRUE;
			}
			if (bIsAddParam == Enum_TRUE)
			{
				/** Remove a last space */
				uCnt--;
				*(pBuffer + uCnt) = 0x00;
			}
			else
			{
				// error handling.
			}
		}

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** path */
	if (pStream->u.msrp.pPath != NULL)
	{
		uLength = 7;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=path:", uLength, uError);
		uCnt += uLength;

		uLength = pal_StringLength(pStream->u.msrp.pPath);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.pPath, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** setup */
	if (pStream->u.msrp.eSetup != ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_NONE)
	{
		uLength = 8;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=setup:", uLength, uError);
		uCnt += uLength;

		if (pStream->u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTIVE)
		{
			uLength = 6;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"active", uLength, uError);
			uCnt += uLength;
		}
		else if (pStream->u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_PASSIVE)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"passive", uLength, uError);
			uCnt += uLength;
		}
		else if (pStream->u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_ACTPASS)
		{
			uLength = 7;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"actpass", uLength, uError);
			uCnt += uLength;
		}
		else if (pStream->u.msrp.eSetup == ECRIO_SDP_MSRP_ATTRIBUTE_SETUP_HOLDCONN)
		{
			// error handling.
		}
		else
		{
			// error handling.
		}

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** file-selector */
	if (pStream->u.msrp.fileSelector.pName != NULL ||
		pStream->u.msrp.fileSelector.uSize != 0 ||
		pStream->u.msrp.fileSelector.pType != NULL ||
		pStream->u.msrp.fileSelector.pHash != NULL)
	{
		uLength = 16;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=file-selector:", uLength, uError);
		uCnt += uLength;

		bIsAddParam = Enum_FALSE;

		if (pStream->u.msrp.fileSelector.pName != NULL)
		{
			uLength = 5;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"name:", uLength, uError);
			uCnt += uLength;

			uLength = pal_StringLength(pStream->u.msrp.fileSelector.pName);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)pStream->u.msrp.fileSelector.pName, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			bIsAddParam = Enum_TRUE;
		}

		if (pStream->u.msrp.fileSelector.uSize != 0)
		{
			uLength = 5;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"size:", uLength, uError);
			uCnt += uLength;

			pal_MemorySet(buf, 0, 32);
			if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d ", pStream->u.msrp.fileSelector.uSize))
			{
				uError = ECRIO_SDP_MEMORY_ERROR;
				goto END;
			}
			uLength = pal_StringLength(buf);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
			uCnt += uLength;

			bIsAddParam = Enum_TRUE;
		}

		if (pStream->u.msrp.fileSelector.pType != NULL)
		{
			uLength = 5;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"type:", uLength, uError);
			uCnt += uLength;

			uLength = pal_StringLength(pStream->u.msrp.fileSelector.pType);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.fileSelector.pType, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			bIsAddParam = Enum_TRUE;
		}

		if (pStream->u.msrp.fileSelector.pHash != NULL)
		{
			uLength = 5;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"hash:", uLength, uError);
			uCnt += uLength;

			uLength = pal_StringLength(pStream->u.msrp.fileSelector.pHash);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.fileSelector.pHash, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			bIsAddParam = Enum_TRUE;
		}

		if (bIsAddParam == Enum_TRUE)
		{
			/** Remove a last space */
			uCnt--;
			*(pBuffer + uCnt) = 0x00;
		}
		else
		{
			// error handling.
		}

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** file-disposition */
	if (pStream->u.msrp.pFileDisposition != NULL)
	{
		uLength = 19;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=file-disposition:", uLength, uError);
		uCnt += uLength;

		uLength = pal_StringLength(pStream->u.msrp.pFileDisposition);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.pFileDisposition, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** file-transfer-id */
	if (pStream->u.msrp.pFileTransferId != NULL)
	{
		uLength = 19;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=file-transfer-id:", uLength, uError);
		uCnt += uLength;

		uLength = pal_StringLength(pStream->u.msrp.pFileTransferId);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.pFileTransferId, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** file-date */
	if (pStream->u.msrp.fileDate.pCreation != NULL ||
		pStream->u.msrp.fileDate.pModification != NULL ||
		pStream->u.msrp.fileDate.pRead != NULL)
	{
		uLength = 12;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=file-date:", uLength, uError);
		uCnt += uLength;

		bIsAddParam = Enum_FALSE;

		if (pStream->u.msrp.fileDate.pCreation != NULL)
		{
			uLength = 9;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"creation:", uLength, uError);
			uCnt += uLength;

			uLength = pal_StringLength(pStream->u.msrp.fileDate.pCreation);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.fileDate.pCreation, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			bIsAddParam = Enum_TRUE;
		}

		if (pStream->u.msrp.fileDate.pModification != NULL)
		{
			uLength = 13;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"modification:", uLength, uError);
			uCnt += uLength;

			uLength = pal_StringLength(pStream->u.msrp.fileDate.pModification);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.fileDate.pModification, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			bIsAddParam = Enum_TRUE;
		}

		if (pStream->u.msrp.fileDate.pRead != NULL)
		{
			uLength = 5;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"read:", uLength, uError);
			uCnt += uLength;

			uLength = pal_StringLength(pStream->u.msrp.fileDate.pRead);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.fileDate.pRead, uLength, uError);
			uCnt += uLength;

			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_SPACE_SYMBOL, uLength, uError);
			uCnt += uLength;

			bIsAddParam = Enum_TRUE;
		}

		if (bIsAddParam == Enum_TRUE)
		{
			/** Remove a last space */
			uCnt--;
			*(pBuffer + uCnt) = 0x00;
		}
		else
		{
			// error handling.
		}

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** file-icon */
	if (pStream->u.msrp.pFileIcon != NULL)
	{
		uLength = 12;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=file-icon:", uLength, uError);
		uCnt += uLength;

		uLength = pal_StringLength(pStream->u.msrp.pFileIcon);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, pStream->u.msrp.pFileIcon, uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** file-range */
	if (pStream->u.msrp.fileRange.uStart != 0)
	{
		uLength = 13;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=file-range:", uLength, uError);
		uCnt += uLength;

		/** start offset */
		pal_MemorySet(buf, 0, 32);
		if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->u.msrp.fileRange.uStart))
		{
			uError = ECRIO_SDP_MEMORY_ERROR;
			goto END;
		}
		uLength = pal_StringLength(buf);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
		uCnt += uLength;

		/** "-" */
		uLength = 1;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_DASH_SYMBOL, uLength, uError);
		uCnt += uLength;

		/** stop offset */
		if (pStream->u.msrp.fileRange.uStart != 0)
		{
			pal_MemorySet(buf, 0, 32);
			if (0 >= pal_StringSNPrintf((char *)buf, 32, "%d", pStream->u.msrp.fileRange.uStop))
			{
				uError = ECRIO_SDP_MEMORY_ERROR;
				goto END;
			}
			uLength = pal_StringLength(buf);
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)buf, uLength, uError);
			uCnt += uLength;
		}
		else
		{
			uLength = 1;
			CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
			SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_ASTERISK_SYMBOL, uLength, uError);
			uCnt += uLength;
		}

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** msrp-cema */
	if (pStream->u.msrp.bCEMA == Enum_TRUE)
	{
		uLength = 11;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=msrp-cema", uLength, uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** chatroom */
	if (pStream->u.msrp.bIsClosed == Enum_TRUE)
	{
		uLength = 2 + pal_StringLength(ECRIO_SDP_MEDIA_CHATROOM_STRING) + 1
			+ pal_StringLength(ECRIO_SDP_CLOSED_GROUPCHAT_STRING);
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, (u_char *)"a=", 2, uError);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt + 2, ECRIO_SDP_MEDIA_CHATROOM_STRING,
			pal_StringLength(ECRIO_SDP_MEDIA_CHATROOM_STRING), uError);

		/** parameter - org.openmobilealliance.groupchat.closed */
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt + 2 + pal_StringLength(ECRIO_SDP_MEDIA_CHATROOM_STRING), ECRIO_SDP_COLON_SYMBOL, 1, uError);
		SDP_STRING_CONCATENATE(pBuffer,
			uContainerSize - uCnt + 2 + pal_StringLength(ECRIO_SDP_MEDIA_CHATROOM_STRING) + 1,
			ECRIO_SDP_CLOSED_GROUPCHAT_STRING,
			pal_StringLength(ECRIO_SDP_CLOSED_GROUPCHAT_STRING),
			uError);
		uCnt += uLength;

		uLength = 2;
		CHECK_FORM_LENGTH(uCnt, uLength, uContainerSize);
		SDP_STRING_CONCATENATE(pBuffer, uContainerSize - uCnt, ECRIO_SDP_CRLF_SYMBOL, uLength, uError);
		uCnt += uLength;
	}

	/** Mode attribute */
	uError = ec_SDP_FormALine_ModeAttribute(pStream, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

END:
	*pCnt = uCnt;
	return uError;
}

/**
 * This is used to form a SDP data from SDP structure.
 *
 * This function does not create a buffer for SDP data, then function caller
 * must prepare an enough buffer and specified a maximum buffer size to pSize.
 *
 * After forming successfully, this function outputs entire SDP data to pBuffer
 * and length of SDP data to pSize.
 *
 * @param[in]		pSdpStruct		Pointer to the SDP structure. Must be non-NULL.
 * @param[in/out]	pBuffer			Pointer to the SDP buffer. Must be non-NULL.
 * @param[in]		uContainerSize	The maximum size of the SDP buffer to allocate.
 * @param[out]		pSize			Pointer to the size of SDP buffer. Must be non-NULL.
 * @param[in]		bIsOffer		If TRUE, there is an offer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPForm
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pBuffer,
	u_int32 uContainerSize,
	u_int32 *pSize,
	BoolEnum bIsOffer
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	u_int32	i;
	u_int32	uCnt = 0;

	/** Parameter check */
	if (pSdpStruct == NULL || pBuffer == NULL || uContainerSize == 0 || pSize == NULL)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	/** Reset buffer */
	pal_MemorySet(pBuffer, 0, uContainerSize);

	/** Create "v=" line */
	uError = ec_SDP_FormVLine(pSdpStruct, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	/** Create "o=" line */
	uError = ec_SDP_FormOLine(&pSdpStruct->origin, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	/** Create "s=" line */
	uError = ec_SDP_FormSLine(pSdpStruct, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	/** Create "c=" line */
	if (pSdpStruct->conn.eAddressType != ECRIO_SDP_ADDRESS_TYPE_NONE &&
		pSdpStruct->conn.pAddress != NULL)
	{
		uError = ec_SDP_FormCLine(&pSdpStruct->conn, pBuffer, uContainerSize, &uCnt);
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	/** Create "b=" line */
	uError = ec_SDP_FormBLine(&pSdpStruct->bandwidth, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	/** Create "t=" line */
	uError = ec_SDP_FormTLine(pSdpStruct, pBuffer, uContainerSize, &uCnt);
	if (uError != ECRIO_SDP_NO_ERROR)
	{
		goto END;
	}

	/** Create Media ("m=") line */
	for (i = 0; i < pSdpStruct->uNumOfMedia; i++)
	{
		/** Currently audio and MSRP types are supported, otherwise skip to create
		  * media (and also audio) line. */
		if (pSdpStruct->stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_AUDIO)
		{
			uError = ec_SDP_FormMLine_Audio(&pSdpStruct->stream[i], pBuffer, uContainerSize, &uCnt, bIsOffer);
		}
		else if (pSdpStruct->stream[i].eMediaType == ECRIO_SDP_MEDIA_TYPE_MSRP)
		{
			uError = ec_SDP_FormMLine_MSRP(&pSdpStruct->stream[i], pBuffer, uContainerSize, &uCnt);
		}
		else
		{
			continue;
		}
	}

	*pSize = uCnt;
END:
	return uError;
}
