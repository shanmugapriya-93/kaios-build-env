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
* @file EcrioSDPParseHeaderA_file-date.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderA_file-date.c" */
static const int ec_sdp_Parse_Header_A_file_date_start = 1;

/* #line 94 "EcrioSDPParseHeaderA_file-date.rl" */


typedef enum
{
	SDP_MSRP_FileDate_None = 0,
	SDP_MSRP_FileDate_Creation,
	SDP_MSRP_FileDate_Modification,
	SDP_MSRP_FileDate_Read,
} ec_SDP_MSRP_FileDateEnum;


ec_SDP_MSRP_FileDateEnum ec_sdp_FindFileDate
(
	u_char *pStr
)
{
	ec_SDP_MSRP_FileDateEnum eDateParam = SDP_MSRP_FileDate_None;

	if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_FILE_DATE_CREATION_STRING,
							pal_StringLength(ECRIO_SDP_MSRP_FILE_DATE_CREATION_STRING)) == 0)
	{
		eDateParam = SDP_MSRP_FileDate_Creation;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_FILE_DATE_MODIFICATION_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_FILE_DATE_MODIFICATION_STRING)) == 0)
	{
		eDateParam = SDP_MSRP_FileDate_Modification;
	}
	else if (pal_StringNCompare(pStr, ECRIO_SDP_MSRP_FILE_DATE_READ_STRING,
								pal_StringLength(ECRIO_SDP_MSRP_FILE_DATE_READ_STRING)) == 0)
	{
		eDateParam = SDP_MSRP_FileDate_Read;
	}
	else
	{
		eDateParam = SDP_MSRP_FileDate_None;
	}

	return eDateParam;
}

u_int32 ec_sdp_FillFileDate
(
	EcrioSDPStringStruct *pStrings,
	EcrioSDPStreamStruct *pStream,
	u_char *pStart,
	u_char *pEnd,
	ec_SDP_MSRP_FileDateEnum eDateParam
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;

	switch (eDateParam)
	{
		case SDP_MSRP_FileDate_Creation:
		{
			uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.fileDate.pCreation, pStart, (pEnd - pStart));
		}
		break;

		case SDP_MSRP_FileDate_Modification:
		{
			uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.fileDate.pModification, pStart, (pEnd - pStart));
		}
		break;

		case SDP_MSRP_FileDate_Read:
		{
			uError = ec_sdp_StringCopy(pStrings, (void **)&pStream->u.msrp.fileDate.pRead, pStart, (pEnd - pStart));
		}
		break;

		default:
			break;
	}

	return uError;
}

/**
 * This function is used to parse the file-date attribute ("a=file-date") in
 * media attribute line and store information to the SDP information structure.
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
u_int32 ec_sdp_ParseFileDate
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
	ec_SDP_MSRP_FileDateEnum	eDateParam = SDP_MSRP_FileDate_None;

	
/* #line 174 "EcrioSDPParseHeaderA_file-date.c" */
	{
	cs = ec_sdp_Parse_Header_A_file_date_start;
	}

/* #line 203 "EcrioSDPParseHeaderA_file-date.rl" */
	
/* #line 181 "EcrioSDPParseHeaderA_file-date.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 33: goto tr1;
		case 37: goto tr1;
		case 39: goto tr1;
		case 126: goto tr1;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr1;
		} else if ( (*p) >= 42 )
			goto tr1;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr1;
		} else if ( (*p) >= 65 )
			goto tr1;
	} else
		goto tr1;
	goto tr0;
tr0:
/* #line 81 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FILE_DATE_LINE_ERROR;
	}
	goto st0;
/* #line 215 "EcrioSDPParseHeaderA_file-date.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 66 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 229 "EcrioSDPParseHeaderA_file-date.c" */
	switch( (*p) ) {
		case 33: goto st2;
		case 37: goto st2;
		case 39: goto st2;
		case 58: goto tr3;
		case 126: goto st2;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st2;
		} else if ( (*p) >= 42 )
			goto st2;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st2;
		} else if ( (*p) >= 65 )
			goto st2;
	} else
		goto st2;
	goto tr0;
tr3:
/* #line 71 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		eDateParam = ec_sdp_FindFileDate((u_char*)tag_start);
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 262 "EcrioSDPParseHeaderA_file-date.c" */
	if ( (*p) == 34 )
		goto tr4;
	goto tr0;
tr4:
/* #line 66 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 276 "EcrioSDPParseHeaderA_file-date.c" */
	if ( (*p) == 32 )
		goto st5;
	if ( (*p) < 48 ) {
		if ( 43 <= (*p) && (*p) <= 45 )
			goto st5;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st5;
		} else if ( (*p) >= 65 )
			goto st5;
	} else
		goto st5;
	goto tr0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 32: goto st5;
		case 34: goto st7;
	}
	if ( (*p) < 48 ) {
		if ( 43 <= (*p) && (*p) <= 45 )
			goto st5;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st5;
		} else if ( (*p) >= 65 )
			goto st5;
	} else
		goto st5;
	goto tr0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 10: goto tr8;
		case 13: goto tr10;
		case 32: goto tr11;
	}
	goto st0;
tr8:
/* #line 76 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		uError = ec_sdp_FillFileDate(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eDateParam);
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 331 "EcrioSDPParseHeaderA_file-date.c" */
	goto st0;
tr10:
/* #line 76 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		uError = ec_sdp_FillFileDate(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eDateParam);
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 343 "EcrioSDPParseHeaderA_file-date.c" */
	if ( (*p) == 10 )
		goto st8;
	goto tr0;
tr11:
/* #line 76 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		uError = ec_sdp_FillFileDate(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eDateParam);
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 357 "EcrioSDPParseHeaderA_file-date.c" */
	switch( (*p) ) {
		case 10: goto st8;
		case 13: goto st6;
		case 33: goto tr1;
		case 37: goto tr1;
		case 39: goto tr1;
		case 126: goto tr1;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr1;
		} else if ( (*p) >= 42 )
			goto tr1;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr1;
		} else if ( (*p) >= 65 )
			goto tr1;
	} else
		goto tr1;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 7: 
/* #line 76 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		uError = ec_sdp_FillFileDate(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eDateParam);
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
/* #line 81 "EcrioSDPParseHeaderA_file-date.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FILE_DATE_LINE_ERROR;
	}
	break;
/* #line 412 "EcrioSDPParseHeaderA_file-date.c" */
	}
	}

	_out: {}
	}

/* #line 204 "EcrioSDPParseHeaderA_file-date.rl" */

	return uError;
}
