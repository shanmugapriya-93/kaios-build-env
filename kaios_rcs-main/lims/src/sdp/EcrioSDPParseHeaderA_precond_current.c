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
* @file EcrioSDPParseHeaderA_precond_current.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"


/* #line 56 "EcrioSDPParseHeaderA_precond_current.c" */
static const int ec_sdp_Parse_Header_A_precond_current_start = 1;
static const int ec_sdp_Parse_Header_A_precond_current_first_final = 7;
static const int ec_sdp_Parse_Header_A_precond_current_error = 0;

static const int ec_sdp_Parse_Header_A_precond_current_en_main = 1;


/* #line 180 "EcrioSDPParseHeaderA_precond_current.rl" */


typedef enum
{
	SDP_MSRP_PrecondLabel_None = 0,
	SDP_MSRP_PrecondLabel_Current,
	SDP_MSRP_PrecondLabel_Confirm,
} ec_SDP_MSRP_PrecondLabelEnum;


/**
 * This function is used to parse the current status attribute ("a=curr") or
 * confirm status attribute ("a=conf") in media attribute line and store
 * information to the SDP information structure.
 * 
 * The parsing string may have a non-NULL terminator, but should be specified
 * proper size to uSize.
 * 
 * @param[out]	pStream			Pointer to the stream structure. Must be non-NULL.
 * @param[in]	bIsCurrent		If TRUE, it works "curr", else "conf".
 * @param[in]	pData			Pointer to the string of format. Must be non-NULL.
 * @param[in]	uSize			The size of parsing string.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 ec_sdp_ParseCurrentStatus
(
	EcrioSDPStreamStruct *pStream,
	BoolEnum bIsCurrent,
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
	ec_SDP_MSRP_PrecondLabelEnum label = (bIsCurrent == Enum_TRUE ? SDP_MSRP_PrecondLabel_Current : SDP_MSRP_PrecondLabel_Confirm);

	/** Note: When parsing the precondition attribute, for convenience, store only "local"
	 *    parameter in "remote" of EcrioSDPPreconditionStruct. This is to consistently
	 *    maintain the direction from myself, therefore, the "remote" parameter (pointed
	 *    to myself) set by other party is not stored and is ignored.
	 */

	
/* #line 113 "EcrioSDPParseHeaderA_precond_current.c" */
	{
	cs = ec_sdp_Parse_Header_A_precond_current_start;
	}

/* #line 228 "EcrioSDPParseHeaderA_precond_current.rl" */
	
/* #line 120 "EcrioSDPParseHeaderA_precond_current.c" */
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
/* #line 167 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_PRECOND_CURRENT_LINE_ERROR;
	}
	goto st0;
/* #line 154 "EcrioSDPParseHeaderA_precond_current.c" */
st0:
cs = 0;
	goto _out;
tr1:
/* #line 65 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		tag_start = p;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
/* #line 168 "EcrioSDPParseHeaderA_precond_current.c" */
	switch( (*p) ) {
		case 32: goto tr2;
		case 33: goto st2;
		case 37: goto st2;
		case 39: goto st2;
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
tr2:
/* #line 70 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_TYPE_QOS_STRING)) != 0)
		{
			uError = ECRIO_SDP_PARSING_A_PRECOND_CURRENT_LINE_ERROR;
			goto END;
		}
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 206 "EcrioSDPParseHeaderA_precond_current.c" */
	switch( (*p) ) {
		case 32: goto st3;
		case 33: goto tr5;
		case 37: goto tr5;
		case 39: goto tr5;
		case 126: goto tr5;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr5;
		} else if ( (*p) >= 42 )
			goto tr5;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr5;
		} else if ( (*p) >= 65 )
			goto tr5;
	} else
		goto tr5;
	goto tr0;
tr5:
/* #line 65 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		tag_start = p;
	}
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
/* #line 239 "EcrioSDPParseHeaderA_precond_current.c" */
	switch( (*p) ) {
		case 32: goto tr6;
		case 33: goto st4;
		case 37: goto st4;
		case 39: goto st4;
		case 126: goto st4;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st4;
		} else if ( (*p) >= 42 )
			goto st4;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st4;
		} else if ( (*p) >= 65 )
			goto st4;
	} else
		goto st4;
	goto tr0;
tr6:
/* #line 80 "EcrioSDPParseHeaderA_precond_current.rl" */
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
			uError = ECRIO_SDP_PARSING_A_PRECOND_CURRENT_LINE_ERROR;
			goto END;
		}
	}
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
/* #line 291 "EcrioSDPParseHeaderA_precond_current.c" */
	switch( (*p) ) {
		case 32: goto st5;
		case 33: goto tr9;
		case 37: goto tr9;
		case 39: goto tr9;
		case 126: goto tr9;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr9;
		} else if ( (*p) >= 42 )
			goto tr9;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto tr9;
		} else if ( (*p) >= 65 )
			goto tr9;
	} else
		goto tr9;
	goto tr0;
tr9:
/* #line 65 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		tag_start = p;
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
/* #line 324 "EcrioSDPParseHeaderA_precond_current.c" */
	switch( (*p) ) {
		case 10: goto tr11;
		case 13: goto tr13;
		case 33: goto st7;
		case 37: goto st7;
		case 39: goto st7;
		case 126: goto st7;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 43 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st7;
		} else if ( (*p) >= 42 )
			goto st7;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 95 <= (*p) && (*p) <= 122 )
				goto st7;
		} else if ( (*p) >= 65 )
			goto st7;
	} else
		goto st7;
	goto st0;
tr11:
/* #line 104 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
			}
		}
		else
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
			}
		}
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
/* #line 416 "EcrioSDPParseHeaderA_precond_current.c" */
	goto st0;
tr13:
/* #line 104 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
			}
		}
		else
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
			}
		}
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
/* #line 486 "EcrioSDPParseHeaderA_precond_current.c" */
	if ( (*p) == 10 )
		goto st8;
	goto tr0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 7: 
/* #line 104 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SENDRECV;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_NONE_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_NONE;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_SEND_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_SEND;
			}
		}
		else if (pal_StringNCompare((u_char*)tag_start, ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING,
								pal_StringLength(ECRIO_SDP_PRECONDITION_DIRECTION_RECV_STRING)) == 0)
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_RECV;
			}
		}
		else
		{
			if (label == SDP_MSRP_PrecondLabel_Current)
			{
				pPrecondStatus->curr.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
			}
			else
			{
				pPrecondStatus->conf.direction = ECRIO_SDP_PRECONDITION_DIRECTION_INVALID;
			}
		}
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
/* #line 167 "EcrioSDPParseHeaderA_precond_current.rl" */
	{
		uError = ECRIO_SDP_PARSING_A_PRECOND_CURRENT_LINE_ERROR;
	}
	break;
/* #line 578 "EcrioSDPParseHeaderA_precond_current.c" */
	}
	}

	_out: {}
	}

/* #line 229 "EcrioSDPParseHeaderA_precond_current.rl" */

END:
	return uError;
}
