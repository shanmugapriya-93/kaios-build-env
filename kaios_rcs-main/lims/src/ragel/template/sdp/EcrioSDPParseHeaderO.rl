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
* @file EcrioSDPParseHeaderO.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

extern EcrioSDPAddressTypeEnum ec_sdp_FindAddressType(u_char *pStr);

%%{
	machine ec_sdp_Parse_Header_O;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;
	DIGIT = 0x30..0x39;

	action tag
	{
		tag_start = p;
	}

	action ParseUsername
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pUsername, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseSessId
	{
		pOrigin->uSessionid = ec_sdp_ConvertToUI64Num((u_char*)tag_start);
	}

	action ParseSessVersion
	{
		pOrigin->uVersion = ec_sdp_ConvertToUI64Num((u_char*)tag_start);
	}

	action ParseNettype
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_ADDR_NETTYPE_INTERNET_STRING,
								pal_StringLength(ECRIO_SDP_ADDR_NETTYPE_INTERNET_STRING)) != 0)
		{
			uError = ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
			goto END;
		}
	}

	action ParseAddrtype
	{
		pOrigin->eAddressType = ec_sdp_FindAddressType((u_char*)tag_start);
	}

	action ParseAddr
	{
		uError = ec_sdp_StringCopy(pStrings, (void**)&pOrigin->pAddress, (u_char*)tag_start, (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_O_LINE_ERROR;
	}

	username = any*>tag %ParseUsername;
	sess_id = DIGIT+>tag %ParseSessId;
	sess_version = DIGIT+>tag %ParseSessVersion;
	nettype = any*>tag %ParseNettype;
	addrtype = any*>tag %ParseAddrtype;
	addr = any*>tag %ParseAddr;

	O = 'o' SP* "=" SP*<: username :>SP sess_id :>SP sess_version :>SP nettype :>SP addrtype :>SP addr :>(CRLF | LF)?;

	main := O @!error;

	write data;
}%%

/**
 * This is used to parse SDP Origin ("o=") line and fill up the SDP structure.
 * 
 * @param[in]	pStrings	Pointer to the string structure. Must be non-NULL.
 * @param[out]	pOrigin		Pointer to the origin structure. Must be non-NULL.
 * @param[in]	pData		Pointer to the SDP Origin ("o=") line. Must be non-NULL.
 * @param[in]	uSize		The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseHeaderO
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPOriginStruct *pOrigin,
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

	%% write init;
	%% write exec;

END:
	return uError;
}
