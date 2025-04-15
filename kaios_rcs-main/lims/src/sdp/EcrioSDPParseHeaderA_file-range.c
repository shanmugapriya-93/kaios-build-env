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
* @file EcrioSDPParseHeaderA_file-range.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderA_file-range.c" */
static const int ec_sdp_Parse_Header_A_file_range_start = 1;

/* #line 99 "EcrioSDPParseHeaderA_file-range.rl" */



/**
 * This function is used to parse the file-range attribute ("a=file-range") in
 * media attribute line and store information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseFileRange
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

	
/* #line 95 "EcrioSDPParseHeaderA_file-range.c" */
	{
	cs = ec_sdp_Parse_Header_A_file_range_start;
	}

/* #line 129 "EcrioSDPParseHeaderA_file-range.rl" */
	
/* #line 102 "EcrioSDPParseHeaderA_file-range.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr1;
	goto tr0;
tr0:
/* #line 87 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FILE_RANGE_LINE_ERROR;
	}
	goto st0;
/* #line 118 "EcrioSDPParseHeaderA_file-range.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 65 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 132 "EcrioSDPParseHeaderA_file-range.c" */
	if ( (*p) == 45 )
		goto tr2;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st2;
	goto tr0;
tr2:
/* #line 70 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		pStream->u.msrp.fileRange.uStart = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 148 "EcrioSDPParseHeaderA_file-range.c" */
	switch( (*p) ) {
		case 10: goto tr5;
		case 13: goto tr7;
		case 42: goto tr8;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr8;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr8;
	} else
		goto tr8;
	goto st0;
tr5:
/* #line 65 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		tag_start = p;
	}
/* #line 75 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"*", 1) == 0)
		{
			pStream->u.msrp.fileRange.uStop = 0;
		}
		else
		{
			pStream->u.msrp.fileRange.uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		}
	}
	goto st5;
tr9:
/* #line 75 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"*", 1) == 0)
		{
			pStream->u.msrp.fileRange.uStop = 0;
		}
		else
		{
			pStream->u.msrp.fileRange.uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		}
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 197 "EcrioSDPParseHeaderA_file-range.c" */
	goto st0;
tr7:
/* #line 65 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		tag_start = p;
	}
/* #line 75 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"*", 1) == 0)
		{
			pStream->u.msrp.fileRange.uStop = 0;
		}
		else
		{
			pStream->u.msrp.fileRange.uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		}
	}
	goto st3;
tr10:
/* #line 75 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"*", 1) == 0)
		{
			pStream->u.msrp.fileRange.uStop = 0;
		}
		else
		{
			pStream->u.msrp.fileRange.uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		}
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 233 "EcrioSDPParseHeaderA_file-range.c" */
	if ( (*p) == 10 )
		goto st5;
	goto tr0;
tr8:
/* #line 65 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		tag_start = p;
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 247 "EcrioSDPParseHeaderA_file-range.c" */
	switch( (*p) ) {
		case 10: goto tr9;
		case 13: goto tr10;
		case 42: goto st6;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st6;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st6;
	} else
		goto st6;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 6: 
/* #line 75 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"*", 1) == 0)
		{
			pStream->u.msrp.fileRange.uStop = 0;
		}
		else
		{
			pStream->u.msrp.fileRange.uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		}
	}
	break;
	case 1: 
	case 2: 
	case 3: 
/* #line 87 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_FILE_RANGE_LINE_ERROR;
	}
	break;
	case 4: 
/* #line 65 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		tag_start = p;
	}
/* #line 75 "EcrioSDPParseHeaderA_file-range.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"*", 1) == 0)
		{
			pStream->u.msrp.fileRange.uStop = 0;
		}
		else
		{
			pStream->u.msrp.fileRange.uStop = (u_int32)pal_StringConvertToUNum((u_char*)tag_start, NULL, 10);
		}
	}
	break;
/* #line 311 "EcrioSDPParseHeaderA_file-range.c" */
	}
	}

	_out: {}
	}

/* #line 130 "EcrioSDPParseHeaderA_file-range.rl" */

	return uError;
}
