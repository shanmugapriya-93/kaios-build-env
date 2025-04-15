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


/* #line 56 "EcrioSDPParseHeaderA_file-selector.c" */
static const int ec_sdp_Parse_Header_A_file_selector_start = 1;

/* #line 93 "EcrioSDPParseHeaderA_file-selector.rl" */


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

	
/* #line 185 "EcrioSDPParseHeaderA_file-selector.c" */
	{
	cs = ec_sdp_Parse_Header_A_file_selector_start;
	}

/* #line 213 "EcrioSDPParseHeaderA_file-selector.rl" */
	
/* #line 192 "EcrioSDPParseHeaderA_file-selector.c" */
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
/* #line 80 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FILE_SELECTOR_LINE_ERROR;
	}
	goto st0;
/* #line 226 "EcrioSDPParseHeaderA_file-selector.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 240 "EcrioSDPParseHeaderA_file-selector.c" */
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
/* #line 70 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		eSelector = ec_sdp_FindFileSelector((u_char*)tag_start);
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 273 "EcrioSDPParseHeaderA_file-selector.c" */
	switch( (*p) ) {
		case 10: goto tr6;
		case 13: goto tr7;
		case 32: goto tr8;
	}
	goto tr5;
tr5:
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 290 "EcrioSDPParseHeaderA_file-selector.c" */
	switch( (*p) ) {
		case 10: goto tr10;
		case 13: goto tr11;
		case 32: goto tr12;
	}
	goto st5;
tr6:
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	goto st6;
tr10:
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	goto st6;
tr17:
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 327 "EcrioSDPParseHeaderA_file-selector.c" */
	goto st0;
tr7:
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	goto st3;
tr11:
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	goto st3;
tr18:
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 359 "EcrioSDPParseHeaderA_file-selector.c" */
	if ( (*p) == 10 )
		goto st6;
	goto tr0;
tr8:
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	goto st7;
tr12:
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	goto st7;
tr19:
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 393 "EcrioSDPParseHeaderA_file-selector.c" */
	switch( (*p) ) {
		case 10: goto tr10;
		case 13: goto tr11;
		case 32: goto tr12;
		case 33: goto tr14;
		case 37: goto tr14;
		case 39: goto tr14;
		case 126: goto tr14;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr14;
		} else if ( (*p) >= 42 )
			goto tr14;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr14;
		} else if ( (*p) >= 65 )
			goto tr14;
	} else
		goto tr14;
	goto st5;
tr14:
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 428 "EcrioSDPParseHeaderA_file-selector.c" */
	switch( (*p) ) {
		case 10: goto tr10;
		case 13: goto tr11;
		case 32: goto tr12;
		case 33: goto st8;
		case 37: goto st8;
		case 39: goto st8;
		case 58: goto tr16;
		case 126: goto st8;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st8;
		} else if ( (*p) >= 42 )
			goto st8;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st8;
		} else if ( (*p) >= 65 )
			goto st8;
	} else
		goto st8;
	goto st5;
tr16:
/* #line 70 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		eSelector = ec_sdp_FindFileSelector((u_char*)tag_start);
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
/* #line 464 "EcrioSDPParseHeaderA_file-selector.c" */
	switch( (*p) ) {
		case 10: goto tr17;
		case 13: goto tr18;
		case 32: goto tr19;
	}
	goto tr5;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 5: 
	case 7: 
	case 8: 
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	break;
	case 1: 
	case 2: 
	case 3: 
/* #line 80 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FILE_SELECTOR_LINE_ERROR;
	}
	break;
	case 4: 
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
	break;
	case 9: 
/* #line 75 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		uError = ec_sdp_FillFileSelector(pStrings, pStream, (u_char*)tag_start, (u_char*)p, eSelector);
	}
/* #line 65 "EcrioSDPParseHeaderA_file-selector.rl" */
	{
		tag_start = p;
	}
	break;
/* #line 521 "EcrioSDPParseHeaderA_file-selector.c" */
	}
	}

	_out: {}
	}

/* #line 214 "EcrioSDPParseHeaderA_file-selector.rl" */

	return uError;
}
