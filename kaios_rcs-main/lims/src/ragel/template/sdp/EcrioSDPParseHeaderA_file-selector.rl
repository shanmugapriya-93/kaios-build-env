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
* @file EcrioSDPParseHeaderA_file-selector.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parse_Header_A_file_selector;

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

	action ParseLabel
	{
		eSelector = ec_sdp_FindFileSelector((u_char*)tag_start);
	}

	action ParseValue
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_A_FILE_SELECTOR_LINE_ERROR;
	}

	label = token>tag %ParseLabel;
	value = any*>tag %ParseValue;

	param = label ":" value;
	selector = param (SP param)* (SP{1})? :>(CRLF | LF)?;

	main := selector @!error;

	write data;
}%%

typedef enum
{
	SDP_MSRP_FileSelector_None = 0,
	SDP_MSRP_FileSelector_Name,
	SDP_MSRP_FileSelector_Size,
	SDP_MSRP_FileSelector_Type,
	SDP_MSRP_FileSelector_Hash,
} ec_SDP_MSRP_FileSelectorEnum;

ec_SDP_MSRP_FileSelectorEnum ec_sdp_FindFileSelector
(
	u_char *pStr
)
{
	ec_SDP_MSRP_FileSelectorEnum eSelector = SDP_MSRP_FileSelector_None;

	if (pal_StringNCompare(pStr, ECRIO_SDP_FILE_SELECTOR_NAME_STRING,
							pal_StringLength(ECRIO_SDP_FILE_SELECTOR_NAME_STRING)) == 0)
	{
		eSelector = SDP_MSRP_FileSelector_Name;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_FILE_SELECTOR_SIZE_STRING,
								pal_StringLength(ECRIO_SDP_FILE_SELECTOR_SIZE_STRING)) == 0)
	{
		eSelector = SDP_MSRP_FileSelector_Size;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_FILE_SELECTOR_TYPE_STRING,
								pal_StringLength(ECRIO_SDP_FILE_SELECTOR_TYPE_STRING)) == 0)
	{
		eSelector = SDP_MSRP_FileSelector_Type;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_FILE_SELECTOR_HASH_STRING,
								pal_StringLength(ECRIO_SDP_FILE_SELECTOR_HASH_STRING)) == 0)
	{
		eSelector = SDP_MSRP_FileSelector_Hash;
	}
	else
	{
		eSelector = SDP_MSRP_FileSelector_None;
	}

	return eSelector;
}

u_int32 ec_sdp_FillFileSelector
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pStream,
	u_char *pStart,
	u_char *pEnd,
	ec_SDP_MSRP_FileSelectorEnum eSelector
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;

	switch (eSelector)
	{
		case SDP_MSRP_FileSelector_Name:
		{
			uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.fileSelector.pName, pStart, (pEnd - pStart));
		}
		break;

		case SDP_MSRP_FileSelector_Size:
		{
			pStream->u.msrp.fileSelector.uSize = (u_int32)pal_StringConvertToUNum(pStart, NULL, 10);
		}
		break;

		case SDP_MSRP_FileSelector_Type:
		{
			uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.fileSelector.pType, pStart, (pEnd - pStart));
		}
		break;

		case SDP_MSRP_FileSelector_Hash:
		{
			uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.fileSelector.pHash, pStart, (pEnd - pStart));
		}
		break;

		default:
			break;
	}

	return uError;
}

/**
 * This function is used to parse the file selector attribute ("a=file-selector")
 * in media attribute line and store information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 * 
 * @param[in]	pStrings		Pointer to the string structure. Must be non-NULL.
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseFileSelector
(
	EcrioSDPStringStruct *pStrings,
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
	ec_SDP_MSRP_FileSelectorEnum	eSelector = SDP_MSRP_FileSelector_None;

	%% write init;
	%% write exec;

	return uError;
}
