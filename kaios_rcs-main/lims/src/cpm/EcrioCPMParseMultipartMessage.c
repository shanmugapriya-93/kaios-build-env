
/* #line 1 "EcrioCPMParseMultipartMessage.rl" */
/******************************************************************************

Copyright (c) 2019-2020 Ecrio, Inc. All Rights Reserved.

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
 * @file EcrioCPMParseMultipartMessage.c
 * @brief Implementation of the CPM module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"


/* #line 56 "EcrioCPMParseMultipartMessage.c" */
static const int ec_cpm_parse_multipart_message_start = 1;
static const int ec_cpm_parse_multipart_message_first_final = 166;
static const int ec_cpm_parse_multipart_message_error = 0;

static const int ec_cpm_parse_multipart_message_en_main = 1;


/* #line 155 "EcrioCPMParseMultipartMessage.rl" */




u_int32 ec_CPM_ParseMIMEMessageBody
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartBodyStruct *pStruct,
	u_char *pData,
	u_int32 uLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_int32 uCntLen = 0;
	s_int32 cs;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	u_char cTemp[8];

	/** Check parameter validity. */
	if (pContext == NULL || pStruct == NULL || pData == NULL || uLen == 0)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	p = (char*)pData;
	pe = (char*)pData + uLen;
	eof = pe;

	/** Parsing MIME message by Ragel */
	
/* #line 100 "EcrioCPMParseMultipartMessage.c" */
	{
	cs = ec_cpm_parse_multipart_message_start;
	}

/* #line 190 "EcrioCPMParseMultipartMessage.rl" */
	
/* #line 107 "EcrioCPMParseMultipartMessage.c" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 13: goto st2;
		case 67: goto tr2;
	}
	goto tr0;
tr0:
/* #line 129 "EcrioCPMParseMultipartMessage.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
	}
	goto st0;
/* #line 129 "EcrioCPMParseMultipartMessage.c" */
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	if ( (*p) == 10 )
		goto st166;
	goto tr0;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
	goto tr187;
tr187:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st167;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
/* #line 155 "EcrioCPMParseMultipartMessage.c" */
	goto st167;
tr2:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 167 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 79: goto st4;
		case 111: goto st34;
	}
	goto tr0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 78 )
		goto st5;
	goto tr0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 84 )
		goto st6;
	goto tr0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 69 )
		goto st7;
	goto tr0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 78 )
		goto st8;
	goto tr0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 84 )
		goto st9;
	goto tr0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 45 )
		goto st10;
	goto tr0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 84 )
		goto st11;
	goto tr0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 82 )
		goto st12;
	goto tr0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 65 )
		goto st13;
	goto tr0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 78 )
		goto st14;
	goto tr0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( (*p) == 83 )
		goto st15;
	goto tr0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 70 )
		goto st16;
	goto tr0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 69 )
		goto st17;
	goto tr0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 82 )
		goto st18;
	goto tr0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 45 )
		goto st19;
	goto tr0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( (*p) == 69 )
		goto st20;
	goto tr0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( (*p) == 78 )
		goto st21;
	goto tr0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( (*p) == 67 )
		goto st22;
	goto tr0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( (*p) == 79 )
		goto st23;
	goto tr0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( (*p) == 68 )
		goto st24;
	goto tr0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( (*p) == 73 )
		goto st25;
	goto tr0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( (*p) == 78 )
		goto st26;
	goto tr0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	if ( (*p) == 71 )
		goto st27;
	goto tr0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	switch( (*p) ) {
		case 9: goto st27;
		case 32: goto st27;
		case 58: goto st28;
	}
	goto tr0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 13: goto st29;
		case 32: goto st28;
		case 37: goto tr31;
		case 61: goto tr31;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr31;
		} else if ( (*p) >= 33 )
			goto tr31;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr31;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr31;
		} else
			goto tr31;
	} else
		goto tr31;
	goto tr0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( (*p) == 10 )
		goto st30;
	goto tr0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( (*p) == 32 )
		goto st31;
	goto tr0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 32: goto st31;
		case 37: goto tr31;
		case 61: goto tr31;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr31;
		} else if ( (*p) >= 33 )
			goto tr31;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr31;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr31;
		} else
			goto tr31;
	} else
		goto tr31;
	goto tr0;
tr31:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
/* #line 423 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr34;
		case 37: goto st32;
		case 61: goto st32;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st32;
		} else if ( (*p) >= 33 )
			goto st32;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st32;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st32;
		} else
			goto st32;
	} else
		goto st32;
	goto tr0;
tr34:
/* #line 104 "EcrioCPMParseMultipartMessage.rl" */
	{
		pStruct->pContentTransferEncoding = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st33;
tr83:
/* #line 87 "EcrioCPMParseMultipartMessage.rl" */
	{
		pStruct->pContentId = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st33;
tr101:
/* #line 92 "EcrioCPMParseMultipartMessage.rl" */
	{
		pal_MemorySet(cTemp, 0, 8);
		if (NULL == pal_StringNCopy(cTemp, 8, (u_char*)tag_start, (p - tag_start)))
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.",
				__FUNCTION__, __LINE__);
			goto END;
		}
		uCntLen = pal_StringToNum(cTemp, NULL);
	}
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
/* #line 471 "EcrioCPMParseMultipartMessage.c" */
	if ( (*p) == 10 )
		goto st1;
	goto tr0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 110 )
		goto st35;
	goto tr0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 116 )
		goto st36;
	goto tr0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	if ( (*p) == 101 )
		goto st37;
	goto tr0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) == 110 )
		goto st38;
	goto tr0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( (*p) == 116 )
		goto st39;
	goto tr0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( (*p) == 45 )
		goto st40;
	goto tr0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 68: goto st41;
		case 73: goto st67;
		case 76: goto st82;
		case 84: goto st93;
	}
	goto tr0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( (*p) == 105 )
		goto st42;
	goto tr0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) == 115 )
		goto st43;
	goto tr0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) == 112 )
		goto st44;
	goto tr0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( (*p) == 111 )
		goto st45;
	goto tr0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( (*p) == 115 )
		goto st46;
	goto tr0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 105 )
		goto st47;
	goto tr0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( (*p) == 116 )
		goto st48;
	goto tr0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( (*p) == 105 )
		goto st49;
	goto tr0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( (*p) == 111 )
		goto st50;
	goto tr0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( (*p) == 110 )
		goto st51;
	goto tr0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	switch( (*p) ) {
		case 9: goto st51;
		case 32: goto st51;
		case 58: goto st52;
	}
	goto tr0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 13: goto st53;
		case 32: goto st52;
		case 37: goto tr59;
		case 61: goto tr59;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr59;
		} else if ( (*p) >= 33 )
			goto tr59;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr59;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr59;
		} else
			goto tr59;
	} else
		goto tr59;
	goto tr0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( (*p) == 10 )
		goto st54;
	goto tr0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 32 )
		goto st55;
	goto tr0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 32: goto st55;
		case 37: goto tr59;
		case 61: goto tr59;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr59;
		} else if ( (*p) >= 33 )
			goto tr59;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr59;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr59;
		} else
			goto tr59;
	} else
		goto tr59;
	goto tr0;
tr59:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st56;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
/* #line 687 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr62;
		case 32: goto st64;
		case 37: goto st56;
		case 59: goto st60;
		case 61: goto st56;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st56;
		} else if ( (*p) >= 33 )
			goto st56;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st56;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st56;
		} else
			goto st56;
	} else
		goto st56;
	goto tr0;
tr62:
/* #line 99 "EcrioCPMParseMultipartMessage.rl" */
	{
		pStruct->pContentDisposition = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st57;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
/* #line 723 "EcrioCPMParseMultipartMessage.c" */
	if ( (*p) == 10 )
		goto st58;
	goto tr0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st59;
		case 67: goto tr2;
	}
	goto tr0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 32: goto st59;
		case 59: goto st60;
	}
	goto tr0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	switch( (*p) ) {
		case 13: goto st61;
		case 32: goto st60;
		case 37: goto st56;
		case 61: goto st56;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st56;
		} else if ( (*p) >= 33 )
			goto st56;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st56;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st56;
		} else
			goto st56;
	} else
		goto st56;
	goto tr0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 10 )
		goto st62;
	goto tr0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 32 )
		goto st63;
	goto tr0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	switch( (*p) ) {
		case 32: goto st63;
		case 37: goto st56;
		case 61: goto st56;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st56;
		} else if ( (*p) >= 33 )
			goto st56;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st56;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st56;
		} else
			goto st56;
	} else
		goto st56;
	goto tr0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	switch( (*p) ) {
		case 13: goto st65;
		case 32: goto st64;
		case 59: goto st60;
	}
	goto tr0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 10 )
		goto st66;
	goto tr0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 32 )
		goto st59;
	goto tr0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( (*p) == 68 )
		goto st68;
	goto tr0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	switch( (*p) ) {
		case 9: goto st68;
		case 32: goto st68;
		case 58: goto st69;
	}
	goto tr0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	switch( (*p) ) {
		case 13: goto st70;
		case 32: goto st69;
		case 37: goto tr76;
		case 60: goto st76;
		case 61: goto tr76;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr76;
		} else if ( (*p) >= 33 )
			goto tr76;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr76;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr76;
		} else
			goto tr76;
	} else
		goto tr76;
	goto tr0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) == 10 )
		goto st71;
	goto tr0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 32 )
		goto st72;
	goto tr0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	switch( (*p) ) {
		case 13: goto st73;
		case 32: goto st72;
		case 37: goto tr76;
		case 60: goto st76;
		case 61: goto tr76;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr76;
		} else if ( (*p) >= 33 )
			goto tr76;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr76;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr76;
		} else
			goto tr76;
	} else
		goto tr76;
	goto tr0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 10 )
		goto st74;
	goto tr0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( (*p) == 32 )
		goto st75;
	goto tr0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	switch( (*p) ) {
		case 32: goto st75;
		case 60: goto st76;
	}
	goto tr0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	switch( (*p) ) {
		case 37: goto tr76;
		case 61: goto tr76;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr76;
		} else if ( (*p) >= 33 )
			goto tr76;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr76;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr76;
		} else
			goto tr76;
	} else
		goto tr76;
	goto tr0;
tr76:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st77;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
/* #line 987 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr83;
		case 37: goto st77;
		case 62: goto tr85;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st77;
		} else if ( (*p) >= 33 )
			goto st77;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 61 <= (*p) && (*p) <= 93 )
				goto st77;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st77;
		} else
			goto st77;
	} else
		goto st77;
	goto tr0;
tr85:
/* #line 87 "EcrioCPMParseMultipartMessage.rl" */
	{
		pStruct->pContentId = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st78;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
/* #line 1021 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto st79;
		case 32: goto st78;
	}
	goto tr0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( (*p) == 10 )
		goto st80;
	goto tr0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st81;
		case 67: goto tr2;
	}
	goto tr0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	switch( (*p) ) {
		case 13: goto st33;
		case 32: goto st81;
	}
	goto tr0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( (*p) == 101 )
		goto st83;
	goto tr0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( (*p) == 110 )
		goto st84;
	goto tr0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	if ( (*p) == 103 )
		goto st85;
	goto tr0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( (*p) == 116 )
		goto st86;
	goto tr0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	if ( (*p) == 104 )
		goto st87;
	goto tr0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 9: goto st87;
		case 32: goto st87;
		case 58: goto st88;
	}
	goto tr0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	switch( (*p) ) {
		case 13: goto st89;
		case 32: goto st88;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr98;
	goto tr0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( (*p) == 10 )
		goto st90;
	goto tr0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	if ( (*p) == 32 )
		goto st91;
	goto tr0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( (*p) == 32 )
		goto st91;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr98;
	goto tr0;
tr98:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st92;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
/* #line 1142 "EcrioCPMParseMultipartMessage.c" */
	if ( (*p) == 13 )
		goto tr101;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st92;
	goto tr0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	switch( (*p) ) {
		case 114: goto st94;
		case 121: goto st109;
	}
	goto tr0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( (*p) == 97 )
		goto st95;
	goto tr0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( (*p) == 110 )
		goto st96;
	goto tr0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( (*p) == 115 )
		goto st97;
	goto tr0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	if ( (*p) == 102 )
		goto st98;
	goto tr0;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	if ( (*p) == 101 )
		goto st99;
	goto tr0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	if ( (*p) == 114 )
		goto st100;
	goto tr0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( (*p) == 45 )
		goto st101;
	goto tr0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	if ( (*p) == 69 )
		goto st102;
	goto tr0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	if ( (*p) == 110 )
		goto st103;
	goto tr0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( (*p) == 99 )
		goto st104;
	goto tr0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	if ( (*p) == 111 )
		goto st105;
	goto tr0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	if ( (*p) == 100 )
		goto st106;
	goto tr0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	if ( (*p) == 105 )
		goto st107;
	goto tr0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( (*p) == 110 )
		goto st108;
	goto tr0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( (*p) == 103 )
		goto st27;
	goto tr0;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	if ( (*p) == 112 )
		goto st110;
	goto tr0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( (*p) == 101 )
		goto st111;
	goto tr0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	switch( (*p) ) {
		case 9: goto st111;
		case 32: goto st111;
		case 58: goto st112;
	}
	goto tr0;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	switch( (*p) ) {
		case 13: goto st113;
		case 32: goto st112;
		case 37: goto tr123;
		case 61: goto tr123;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr123;
		} else if ( (*p) >= 33 )
			goto tr123;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr123;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr123;
		} else
			goto tr123;
	} else
		goto tr123;
	goto tr0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( (*p) == 10 )
		goto st114;
	goto tr0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( (*p) == 32 )
		goto st115;
	goto tr0;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	switch( (*p) ) {
		case 32: goto st115;
		case 37: goto tr123;
		case 61: goto tr123;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr123;
		} else if ( (*p) >= 33 )
			goto tr123;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr123;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr123;
		} else
			goto tr123;
	} else
		goto tr123;
	goto tr0;
tr123:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st116;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
/* #line 1365 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr126;
		case 32: goto tr127;
		case 37: goto st116;
		case 59: goto tr129;
		case 61: goto st116;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st116;
		} else if ( (*p) >= 33 )
			goto st116;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st116;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st116;
		} else
			goto st116;
	} else
		goto st116;
	goto tr0;
tr126:
/* #line 82 "EcrioCPMParseMultipartMessage.rl" */
	{
		pStruct->pContentType = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st117;
tr150:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st117;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
/* #line 1406 "EcrioCPMParseMultipartMessage.c" */
	if ( (*p) == 10 )
		goto st118;
	goto tr0;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st119;
		case 67: goto tr2;
	}
	goto tr0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	switch( (*p) ) {
		case 32: goto st119;
		case 59: goto st120;
	}
	goto tr0;
tr129:
/* #line 82 "EcrioCPMParseMultipartMessage.rl" */
	{
		pStruct->pContentType = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st120;
tr153:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st120;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
/* #line 1444 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto st121;
		case 32: goto st120;
		case 37: goto st124;
		case 61: goto st124;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st124;
		} else if ( (*p) >= 33 )
			goto st124;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st124;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st124;
		} else
			goto st124;
	} else
		goto st124;
	goto tr0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	if ( (*p) == 10 )
		goto st122;
	goto tr0;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	if ( (*p) == 32 )
		goto st123;
	goto tr0;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
	switch( (*p) ) {
		case 32: goto st123;
		case 37: goto st124;
		case 61: goto st124;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st124;
		} else if ( (*p) >= 33 )
			goto st124;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st124;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st124;
		} else
			goto st124;
	} else
		goto st124;
	goto tr0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	switch( (*p) ) {
		case 13: goto st125;
		case 32: goto st139;
		case 37: goto st124;
		case 61: goto st140;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st124;
		} else if ( (*p) >= 33 )
			goto st124;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st124;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st124;
		} else
			goto st124;
	} else
		goto st124;
	goto tr0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( (*p) == 10 )
		goto st126;
	goto tr0;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	if ( (*p) == 32 )
		goto st127;
	goto tr0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	switch( (*p) ) {
		case 32: goto st127;
		case 61: goto st128;
	}
	goto tr0;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
	switch( (*p) ) {
		case 13: goto st129;
		case 32: goto st128;
		case 37: goto tr144;
		case 61: goto tr144;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
	if ( (*p) == 10 )
		goto st130;
	goto tr0;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	if ( (*p) == 32 )
		goto st131;
	goto tr0;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
	switch( (*p) ) {
		case 13: goto st132;
		case 32: goto st131;
		case 37: goto tr144;
		case 61: goto tr144;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	if ( (*p) == 10 )
		goto st133;
	goto tr0;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
	if ( (*p) == 32 )
		goto st134;
	goto tr0;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
	switch( (*p) ) {
		case 32: goto st134;
		case 37: goto tr144;
		case 61: goto tr144;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
tr144:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st135;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
/* #line 1682 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr150;
		case 32: goto tr151;
		case 37: goto st135;
		case 59: goto tr153;
		case 61: goto st135;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st135;
		} else if ( (*p) >= 33 )
			goto st135;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st135;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st135;
		} else
			goto st135;
	} else
		goto st135;
	goto tr0;
tr127:
/* #line 82 "EcrioCPMParseMultipartMessage.rl" */
	{
		pStruct->pContentType = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}
	goto st136;
tr151:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st136;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
/* #line 1723 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto st137;
		case 32: goto st136;
		case 59: goto st120;
	}
	goto tr0;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
	if ( (*p) == 10 )
		goto st138;
	goto tr0;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
	if ( (*p) == 32 )
		goto st119;
	goto tr0;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	switch( (*p) ) {
		case 13: goto st125;
		case 32: goto st139;
		case 61: goto st128;
	}
	goto tr0;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
	switch( (*p) ) {
		case 13: goto st141;
		case 32: goto st151;
		case 37: goto tr159;
		case 61: goto tr160;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr159;
		} else if ( (*p) >= 33 )
			goto tr159;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr159;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr159;
		} else
			goto tr159;
	} else
		goto tr159;
	goto tr0;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
	if ( (*p) == 10 )
		goto st142;
	goto tr0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	if ( (*p) == 32 )
		goto st143;
	goto tr0;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
	switch( (*p) ) {
		case 13: goto st132;
		case 32: goto st143;
		case 37: goto tr144;
		case 61: goto tr163;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
tr163:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st144;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
/* #line 1834 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr164;
		case 32: goto tr165;
		case 37: goto tr144;
		case 59: goto tr153;
		case 61: goto tr144;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
tr164:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st145;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
/* #line 1869 "EcrioCPMParseMultipartMessage.c" */
	if ( (*p) == 10 )
		goto st146;
	goto tr0;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st147;
		case 67: goto tr2;
	}
	goto tr0;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
	switch( (*p) ) {
		case 13: goto st132;
		case 32: goto st147;
		case 37: goto tr144;
		case 59: goto st120;
		case 61: goto tr144;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
tr165:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st148;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
/* #line 1921 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto st149;
		case 32: goto st148;
		case 37: goto tr144;
		case 59: goto st120;
		case 61: goto tr144;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
	if ( (*p) == 10 )
		goto st150;
	goto tr0;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	if ( (*p) == 32 )
		goto st147;
	goto tr0;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	switch( (*p) ) {
		case 13: goto st141;
		case 32: goto st151;
		case 37: goto tr144;
		case 61: goto tr163;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
tr159:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st152;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
/* #line 1999 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr171;
		case 32: goto tr172;
		case 37: goto st152;
		case 59: goto tr153;
		case 61: goto st159;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st152;
		} else if ( (*p) >= 33 )
			goto st152;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto st152;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st152;
		} else
			goto st152;
	} else
		goto st152;
	goto tr0;
tr171:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st153;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
/* #line 2034 "EcrioCPMParseMultipartMessage.c" */
	if ( (*p) == 10 )
		goto st154;
	goto tr0;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st155;
		case 67: goto tr2;
	}
	goto tr0;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	switch( (*p) ) {
		case 32: goto st155;
		case 59: goto st120;
		case 61: goto st128;
	}
	goto tr0;
tr172:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st156;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
/* #line 2067 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto st157;
		case 32: goto st156;
		case 59: goto st120;
		case 61: goto st128;
	}
	goto tr0;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	if ( (*p) == 10 )
		goto st158;
	goto tr0;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	if ( (*p) == 32 )
		goto st155;
	goto tr0;
tr160:
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
	goto st159;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
/* #line 2099 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto tr180;
		case 32: goto tr181;
		case 37: goto tr159;
		case 59: goto tr153;
		case 61: goto tr160;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr159;
		} else if ( (*p) >= 33 )
			goto tr159;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr159;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr159;
		} else
			goto tr159;
	} else
		goto tr159;
	goto tr0;
tr180:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st160;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
/* #line 2134 "EcrioCPMParseMultipartMessage.c" */
	if ( (*p) == 10 )
		goto st161;
	goto tr0;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st162;
		case 67: goto tr2;
	}
	goto tr0;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
	switch( (*p) ) {
		case 13: goto st132;
		case 32: goto st162;
		case 37: goto tr144;
		case 59: goto st120;
		case 61: goto tr163;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
tr181:
/* #line 78 "EcrioCPMParseMultipartMessage.rl" */
	{
	}
	goto st163;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
/* #line 2186 "EcrioCPMParseMultipartMessage.c" */
	switch( (*p) ) {
		case 13: goto st164;
		case 32: goto st163;
		case 37: goto tr144;
		case 59: goto st120;
		case 61: goto tr163;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr144;
		} else if ( (*p) >= 33 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 63 <= (*p) && (*p) <= 93 )
				goto tr144;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr144;
		} else
			goto tr144;
	} else
		goto tr144;
	goto tr0;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
	if ( (*p) == 10 )
		goto st165;
	goto tr0;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
	if ( (*p) == 32 )
		goto st162;
	goto tr0;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof166: cs = 166; goto _test_eof; 
	_test_eof167: cs = 167; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof130: cs = 130; goto _test_eof; 
	_test_eof131: cs = 131; goto _test_eof; 
	_test_eof132: cs = 132; goto _test_eof; 
	_test_eof133: cs = 133; goto _test_eof; 
	_test_eof134: cs = 134; goto _test_eof; 
	_test_eof135: cs = 135; goto _test_eof; 
	_test_eof136: cs = 136; goto _test_eof; 
	_test_eof137: cs = 137; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
	_test_eof149: cs = 149; goto _test_eof; 
	_test_eof150: cs = 150; goto _test_eof; 
	_test_eof151: cs = 151; goto _test_eof; 
	_test_eof152: cs = 152; goto _test_eof; 
	_test_eof153: cs = 153; goto _test_eof; 
	_test_eof154: cs = 154; goto _test_eof; 
	_test_eof155: cs = 155; goto _test_eof; 
	_test_eof156: cs = 156; goto _test_eof; 
	_test_eof157: cs = 157; goto _test_eof; 
	_test_eof158: cs = 158; goto _test_eof; 
	_test_eof159: cs = 159; goto _test_eof; 
	_test_eof160: cs = 160; goto _test_eof; 
	_test_eof161: cs = 161; goto _test_eof; 
	_test_eof162: cs = 162; goto _test_eof; 
	_test_eof163: cs = 163; goto _test_eof; 
	_test_eof164: cs = 164; goto _test_eof; 
	_test_eof165: cs = 165; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 167: 
/* #line 109 "EcrioCPMParseMultipartMessage.rl" */
	{
		if (uCntLen == 0 || uCntLen > (p - tag_start))
		{
			uCntLen = p - tag_start;
		}

		pal_MemoryAllocate(uCntLen + 1, (void **)&pStruct->pMessageBody);
		if (pStruct->pMessageBody == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct->pMessageBody, 0, uCntLen + 1);
		pal_MemoryCopy(pStruct->pMessageBody, uCntLen + 1, (u_char*)tag_start, uCntLen);
		pStruct->uContentLength = uCntLen;
	}
	break;
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
	case 7: 
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
	case 26: 
	case 27: 
	case 28: 
	case 29: 
	case 30: 
	case 31: 
	case 32: 
	case 33: 
	case 34: 
	case 35: 
	case 36: 
	case 37: 
	case 38: 
	case 39: 
	case 40: 
	case 41: 
	case 42: 
	case 43: 
	case 44: 
	case 45: 
	case 46: 
	case 47: 
	case 48: 
	case 49: 
	case 50: 
	case 51: 
	case 52: 
	case 53: 
	case 54: 
	case 55: 
	case 56: 
	case 57: 
	case 58: 
	case 59: 
	case 60: 
	case 61: 
	case 62: 
	case 63: 
	case 64: 
	case 65: 
	case 66: 
	case 67: 
	case 68: 
	case 69: 
	case 70: 
	case 71: 
	case 72: 
	case 73: 
	case 74: 
	case 75: 
	case 76: 
	case 77: 
	case 78: 
	case 79: 
	case 80: 
	case 81: 
	case 82: 
	case 83: 
	case 84: 
	case 85: 
	case 86: 
	case 87: 
	case 88: 
	case 89: 
	case 90: 
	case 91: 
	case 92: 
	case 93: 
	case 94: 
	case 95: 
	case 96: 
	case 97: 
	case 98: 
	case 99: 
	case 100: 
	case 101: 
	case 102: 
	case 103: 
	case 104: 
	case 105: 
	case 106: 
	case 107: 
	case 108: 
	case 109: 
	case 110: 
	case 111: 
	case 112: 
	case 113: 
	case 114: 
	case 115: 
	case 116: 
	case 117: 
	case 118: 
	case 119: 
	case 120: 
	case 121: 
	case 122: 
	case 123: 
	case 124: 
	case 125: 
	case 126: 
	case 127: 
	case 128: 
	case 129: 
	case 130: 
	case 131: 
	case 132: 
	case 133: 
	case 134: 
	case 135: 
	case 136: 
	case 137: 
	case 138: 
	case 139: 
	case 140: 
	case 141: 
	case 142: 
	case 143: 
	case 144: 
	case 145: 
	case 146: 
	case 147: 
	case 148: 
	case 149: 
	case 150: 
	case 151: 
	case 152: 
	case 153: 
	case 154: 
	case 155: 
	case 156: 
	case 157: 
	case 158: 
	case 159: 
	case 160: 
	case 161: 
	case 162: 
	case 163: 
	case 164: 
	case 165: 
/* #line 129 "EcrioCPMParseMultipartMessage.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
	}
	break;
	case 166: 
/* #line 73 "EcrioCPMParseMultipartMessage.rl" */
	{
		tag_start = p;
	}
/* #line 109 "EcrioCPMParseMultipartMessage.rl" */
	{
		if (uCntLen == 0 || uCntLen > (p - tag_start))
		{
			uCntLen = p - tag_start;
		}

		pal_MemoryAllocate(uCntLen + 1, (void **)&pStruct->pMessageBody);
		if (pStruct->pMessageBody == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct->pMessageBody, 0, uCntLen + 1);
		pal_MemoryCopy(pStruct->pMessageBody, uCntLen + 1, (u_char*)tag_start, uCntLen);
		pStruct->uContentLength = uCntLen;
	}
	break;
/* #line 2616 "EcrioCPMParseMultipartMessage.c" */
	}
	}

	_out: {}
	}

/* #line 191 "EcrioCPMParseMultipartMessage.rl" */

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

u_int32 ec_CPM_ParseMultipartMessage
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMMultipartMessageStruct **ppStruct,
	char *pBoundary,
	char *pData,
	u_int32 uLen
)
{
	EcrioCPMMultipartMessageStruct *pStruct = NULL;
	EcrioCPMMultipartBodyStruct *pBody = NULL;
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	u_int32 i;
	char *pCur = NULL, *pStart = NULL, *pEnd = NULL;
	char *pWork = NULL;

	/** Check parameter validity. */
	if (pContext == NULL || ppStruct == NULL || pBoundary == NULL || pData == NULL || uLen == 0)
	{
		return  ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartMessageStruct), (void **)&pStruct);
	if (pStruct == NULL)
	{
		uCPMError = ECRIO_CPM_MEMORY_ERROR;

		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
			__FUNCTION__, __LINE__, uCPMError);
		goto END;
	}
	pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMMultipartMessageStruct));

	pCur = pData;

	/** Initialize output buffer structure */
	pStruct->uNumOfBody = 0;

	/** We assumed that message only contains SDP and resource-list. */
	for (i = 0; i < 2; i++)
	{
		/* 1. Find boundary string */
		pStart = (char*)pal_StringFindSubString((u_char*)pCur, (u_char*)pBoundary);
		if (pStart == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		/** 2. pStart pointer goes to boundary + crlf, there is head of MIME message body. */
		pStart = pStart + pal_StringLength((u_char*)pBoundary) + 2;

		if (uLen <= (pStart - pData))
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tInvalid position of pStart.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		/** 3. Find next boundary, 4 bytes ("--" string + crlf) before of pEnd is tail of MIME message body. */
		pEnd = (char*)pal_MemorySearch(pStart, uLen - (pStart - pData), (u_char*)pBoundary, pal_StringLength((u_char*)pBoundary));

		if (pEnd == NULL)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tboundary not found.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}
		pCur = pEnd;
		pEnd -= 2;

		/** 3.1. Check the last crlf */
		if (*(pEnd - 1) == '\n')
		{
			pEnd--;
		}
		if (*(pEnd - 1) == '\r')
		{
			pEnd--;
		}

		/** 4. Copy MIME message to working buffer. */
		pal_MemoryAllocate((u_int32)(pEnd - pStart + 1), (void **)&pWork);
		if (pWork == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto Error;
		}
		pal_MemorySet((void*)pWork, 0, (pEnd - pStart + 1));
		pal_MemoryCopy((void*)pWork, (pEnd - pStart + 1), (const void*)pStart, (u_int32)(pEnd - pStart));

		if (pStruct->uNumOfBody == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartBodyStruct*), (void **)&pStruct->ppBody);
		}
		else
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pStruct->uNumOfBody, 1) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto Error;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pStruct->uNumOfBody + 1), sizeof(EcrioCPMMultipartBodyStruct*)) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto Error;
			}
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfBody + 1) * sizeof(EcrioCPMMultipartBodyStruct*)), (void **)&pStruct->ppBody);
		}

		if (pStruct->ppBody == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto Error;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMMultipartBodyStruct), (void **)&pBody);
		if (pBody == NULL)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto Error;
		}
		pal_MemorySet((void*)pBody, 0, sizeof(EcrioCPMMultipartBodyStruct));

		pStruct->ppBody[pStruct->uNumOfBody] = pBody;
		pStruct->uNumOfBody++;

		/** 5. Parse MIME message */
		uCPMError = ec_CPM_ParseMIMEMessageBody(pContext, pBody, (u_char*)pWork, (pEnd - pStart));
		if (uCPMError != ECRIO_CPM_NO_ERROR)
		{
			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tParsing MIME message error.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_MESSAGE_PARSING_ERROR;
			goto Error;
		}

		pal_MemoryFree((void**)&pWork);
		pWork = NULL;

		/** 6. Check whether the next boundary indicates a term boundary (exists "--" to bottom). */
		if (pal_StringNCompare((u_char*)pCur + pal_StringLength((u_char*)pBoundary), (u_char*)"--", 2) == 0)
		{
			CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tFound final boundary.", __FUNCTION__, __LINE__);
			uCPMError = ECRIO_CPM_NO_ERROR;
			break;
		}
	}

	*ppStruct = pStruct;
	goto END;

Error:
	ec_CPM_ReleaseMultiPartStruct(&pStruct, Enum_TRUE);

END:
	if (pWork != NULL)
	{
		pal_MemoryFree((void**)&pWork);
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

void ec_CPM_ReleaseMultiPartStruct
(
	EcrioCPMMultipartMessageStruct **ppStruct,
	BoolEnum release
)
{
	u_int32 i;
	EcrioCPMMultipartMessageStruct *pStruct;

	if (ppStruct == NULL || *ppStruct == NULL)
	{
		return;
	}

	pStruct = *ppStruct;

	for (i = 0; i < pStruct->uNumOfBody; i++)
	{
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentType);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentId);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentDisposition);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pContentTransferEncoding);
		pal_MemoryFree((void**)&pStruct->ppBody[i]->pMessageBody);
		pal_MemoryFree((void**)&pStruct->ppBody[i]);
	}
	pal_MemoryFree((void**)&pStruct->ppBody);

	if (release == Enum_TRUE)
	{
		pal_MemoryFree((void**)ppStruct);
	}
}
