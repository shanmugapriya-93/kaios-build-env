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
* @file EcrioSDPParseHeaderA_accept-types.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parse_Header_A_accept_types;

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

	action ParseMediaTypes
	{
		*pAcceptTypes |= ec_sdp_FindMSRPMediaTypes((u_char*)tag_start);
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_A_ACCEPT_TYPES_LINE_ERROR;
	}

	format = (token ("/" token)*)>tag %ParseMediaTypes;

	types = format (SP* format)* (SP{1})? :>(CRLF | LF)?;

	main := types @!error;

	write data;
}%%

EcrioSDPMSRPMediaTypesEnum ec_sdp_FindMSRPMediaTypes
(
	u_char *pStr
)
{
	EcrioSDPMSRPMediaTypesEnum eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_NONE;

	if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN_STRING,
							pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_TEXT_PLAIN;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE_STRING,
		pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_CONFERENCE;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_CPIM;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_IMDN_XML;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_IM_ISCOMPOSING_XML;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_MULTIPART_MIXED;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_HTTP_FILE_TRANSFER;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_PUSH_LOCATION;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_MESSAGE_REVOKE;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_PRIVACY_MANAGEMENT;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_LINK_REPORT;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_SPAM_REPORT;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_RICHCARD;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTED_CHIPLIST;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_SUGGESTION_RESPONSE;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_SHARED_CLIENT_DATA;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA_STRING)) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_GROUPDATA;
	}
	else if (pal_StringNCompare(pStr, (u_char*)"*", 1) == 0)
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_WILDCARD;
	}
	else
	{
		eMSRPMediaTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_NONE;
	}

	return eMSRPMediaTypes;
}

/**
 * This function is used to parse the supported media types ("a=accept-types") in
 * media attribute line and store information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize. If string has multiple types, then out parameters will
 * indicate multiple types by bitmask.
 * 
 * @param[out]	pAcceptTypes	Pointer to the u_int32 variable which indicates to 
 *								the accept-types, it will be EcrioSDPMSRPMediaTypesEnum
 *								enumeration. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseAcceptTypes
(
	u_int32 *pAcceptTypes,
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

	*pAcceptTypes = ECRIO_SDP_MSRP_MEDIA_TYPE_NONE;

	%% write init;
	%% write exec;

	return uError;
}
