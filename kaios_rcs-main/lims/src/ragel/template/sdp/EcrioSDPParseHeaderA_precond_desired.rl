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
* @file EcrioSDPParseHeaderA_precond_desired.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parse_Header_A_precond_desired;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;
	DIGIT = 0x30..0x39;
	ALPHA = 0x41..0x5a | 0x61..0x7a;
	alphanum = ALPHA | DIGIT;
	token = ( alphanum | "-" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" )+;

	action tag
	{
		tag_start = p;
	}

	action ParsePrecondType
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING)) != 0)
		{
			uError = ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR;
			goto END;
		}
	}

	action ParseStrengthType
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_MANDATORY;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_OPTIONAL;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_NONE_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_NONE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_FAILURE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN_STRING)) == 0)
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_UNKNOWN;
		}
		else
		{
			strength = ECRIO_SDP_PRECONDITION_STRENGTH_INVALID;
		}
	}

	action ParseStatusType
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_E2E_STRING)) == 0)
		{
			pPrecondStatus = &pStream->precond.e2e;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_LOCAL_STRING)) == 0)
		{
			pPrecondStatus = &pStream->precond.local;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_STATUS_REMOTE_STRING)) == 0)
		{
			pPrecondStatus = &pStream->precond.remote;
		}
		else
		{
			uError = ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR;
			goto END;
		}
	}

	action ParseDirection
	{
		pPrecondStatus->des.strength = strength;
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING)) == 0)
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
		}
		else
		{
			pPrecondStatus->des.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
		}
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_A_PRECOND_DESIRED_LINE_ERROR;
	}

	precondtype = token>tag %ParsePrecondType;
	strengthtype = token>tag %ParseStrengthType;
	statustype = token>tag %ParseStatusType;
	direction = token>tag %ParseDirection;

	desired = precondtype SP* <: strengthtype SP* <: statustype SP* <: direction (CRLF | LF)?;

	main := desired @!error;

	write data;
}%%


/**
 * This function is used to parse the desire status attribute ("a=des")in media
 * attribute line and store information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseDesiredStatus
(
	EcrioSDPStreamStruct *pStream,
	u_char *pData,
	u_int32 uSize
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	s_int32	cs;
	const char	*p = (char*)pData;
	const char	*pe = (char*)pData + uSize;
	const char	*eof = pe;
	const char	*tag_start = NULL;
	EcrioSDPPreconditionStatusStruct *pPrecondStatus = NULL;
	EcrioSDPPreconditionStrengthEnum strength = ECRIO_SDP_PRECONDITION_STRENGTH_INVALID;

	%% write init;
	%% write exec;

END:
	return uError;
}
