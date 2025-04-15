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
* @file EcrioSDPParseHeaderA_DTMF_parameters.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parse_Header_A_DTMF_parameters;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;
	DIGIT = 0x30..0x39;

	action tag
	{
		tag_start = p;
	}

	action ParseStartDigit
	{
		uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStart << 24;
	}

	action ParseStopDigit
	{
		uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		pPayload->u.audio.uSettings |= uStop << 16;
	}

	action error
	{
		uError = ECRIO_SDP_PARSING_A_FMTP_LINE_ERROR;
	}

	startDigit = DIGIT+>tag %ParseStartDigit;
	stopDigit = DIGIT+>tag %ParseStopDigit;

# We currently support only a single range of digits such as '0-11' or '0-15'.
#	format = startDigit "-" stopDigit :>(CRLF | LF)?;

# If we support multiple range of digits in the future, use following parser; Note we should change tag name properly, and need to care about assignments value.
	parameters = startDigit (SP* "-" SP* stopDigit)* SP*;
	format = parameters (SP* "," SP* parameters)* (","{1})? :>(CRLF | LF)?; # if the last "," (separator) remains... it may be a bug.

	main := format @!error;

	write data;
}%%

/**
 * This function is used to parse DTMF specific parameters in a particular
 * format ("a=fmtp") media attribute line and store DTMF parameters to the
 * SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, and pData should be starting
 * AMR specific paramter, coming after format string. It should be specified
 * proper size to uSize. For example, in case of parsing the following part of
 * "a=fmtp" line:
 *   a=fmtp:108 0-15,66,77<CR><LF>...
 * Then pData should be indicated pointer of a head of DTMF parameter that
 * after the format (fmtp:110) string.
 * string and uSize should be set 10 which do not include a CRLF part.
 * 
 * @param[out]	pPayload		Pointer to the RTP payload structure. Must be non-NULL.
 * @param[in]	pData			Pointer to an DTMF Parameter in "a=fmtp" line. Must
 *                              be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseDTMFParameters
(
	EcrioSDPPayloadStruct *pPayload,
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
	u_int32 uStart = 0;
	u_int32 uStop = 0;

	pPayload->u.audio.uSettings = 0;

	%% write init;
	%% write exec;

	return uError;
}
