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
* @file EcrioSDPParseHeaderB.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parse_Header_B;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;
	ALPHA = 0x41..0x5a | 0x61..0x7a;
	DIGIT = 0x30..0x39;
	alphanum = ALPHA | DIGIT;
	token = ( alphanum | "-" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" )+;

	action tag
	{
		tag_start = p;
	}

	action ParseBwtype
	{
		eBwtype = ec_sdp_FindBandwidth((u_char*)tag_start);
	}

	action ParseBandwidth
	{
		uError = ec_sdp_FillBandwidth(pBandwidth, (u_char*)tag_start, eBwtype);
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_B_LINE_ERROR;
	}

	bwtype = token>tag %ParseBwtype;
	bandwidth = DIGIT+>tag %ParseBandwidth;

	B = 'b' SP* "=" SP*<: bwtype ":" bandwidth :>(CRLF | LF)?;

	main := B @!error;

	write data;
}%%

EcrioSDPBandwidthModifierEnum ec_sdp_FindBandwidth
(
	u_char *pStr
)
{
	EcrioSDPBandwidthModifierEnum eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_NONE;

	if (pal_StringNCompare(pStr, ECRIO_SDP_BANDWIDTH_AS_STRING,
							pal_StringLength(ECRIO_SDP_BANDWIDTH_AS_STRING)) == 0)
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_AS;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_BANDWIDTH_RS_STRING,
								pal_StringLength(ECRIO_SDP_BANDWIDTH_RS_STRING)) == 0)
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_RS;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_BANDWIDTH_RR_STRING,
								pal_StringLength(ECRIO_SDP_BANDWIDTH_RR_STRING)) == 0)
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_RR;
	}
	else
	{
		eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_NONE;
	}

	return eBwtype;
}

u_int32 ec_sdp_FillBandwidth
(
	EcrioSDPBandwidthStruct *pBandwidth,
	u_char *pStart,
	EcrioSDPBandwidthModifierEnum eBwtype
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;

	pBandwidth->uModifier |= eBwtype;
	switch (eBwtype)
	{
		case ECRIO_SDP_BANDWIDTH_MODIFIER_AS:
		{
			pBandwidth->uAS = pal_StringConvertToUNum(pStart, NULL, 10);
		}
		break;

		case ECRIO_SDP_BANDWIDTH_MODIFIER_RS:
		{
			pBandwidth->uRS = pal_StringConvertToUNum(pStart, NULL, 10);
		}
		break;

		case ECRIO_SDP_BANDWIDTH_MODIFIER_RR:
		{
			pBandwidth->uRR = pal_StringConvertToUNum(pStart, NULL, 10);
		}
		break;

		default:
			break;
	}

	return uError;
}

/**
 * This is used to parse SDP bandwidth ("b=") line and fill up the SDP structure.
 * 
 * @param[out]	pBandwidth		Pointer to the bandwidth structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP bandwidth ("b=") line. 
 * 								Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderB
(
	EcrioSDPBandwidthStruct *pBandwidth,
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
	EcrioSDPBandwidthModifierEnum	eBwtype = ECRIO_SDP_BANDWIDTH_MODIFIER_NONE;

	%% write init;
	%% write exec;

	return uError;
}
