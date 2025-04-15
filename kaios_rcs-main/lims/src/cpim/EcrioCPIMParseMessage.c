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
 * @file EcrioParseMessage.c
 * @brief Implementation of the CPIM module parsing MIME message functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPIM.h"
#include "EcrioCPIMCommon.h"


/* #line 56 "EcrioCPIMParseMessage.c" */
static const int ec_cpim_MIMEMessageParser_start = 1;
static const int ec_cpim_MIMEMessageParser_first_final = 122;
static const int ec_cpim_MIMEMessageParser_error = 0;

static const int ec_cpim_MIMEMessageParser_en_main = 1;


/* #line 247 "EcrioCPIMParseMessage.rl" */




u_int32 ec_cpim_ParseMIMEMessage
(
	EcrioCPIMStruct *c,
	CPIMMessageStruct *pStruct,
	u_char *pData,
	u_int32 uLen,
	u_int32 uIndex
)
{
	u_int32 uCPIMError = ECRIO_CPIM_NO_ERROR;
	u_int32 uCntLen = 0;
	s_int32 cs;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	u_char cTemp[8];
	u_char *pContentType = NULL;
	u_char *pContentDisposition = NULL;
	u_char *pContentId = NULL;
	u_char *pContentTransferEncoding = NULL;
	CPIMContentTypeEnum eConType = CPIMContentType_None;

	/** Check parameter validity. */
	if (c == NULL)
	{
		return  ECRIO_CPIM_INVALID_HANDLE;
	}

	if (pStruct == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_2;
	}

	if (pData == NULL)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_3;
	}

	if (uLen == 0)
	{
		return ECRIO_CPIM_INVALID_PARAMETER_4;
	}

	CPIMLOGI(c->logHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);

	p = (char*)pData;
	pe = (char*)pData + uLen;
	eof = pe;

	/** Parsing MIME message by Ragel */
	
/* #line 118 "EcrioCPIMParseMessage.c" */
	{
	cs = ec_cpim_MIMEMessageParser_start;
	}

/* #line 300 "EcrioCPIMParseMessage.rl" */
	
/* #line 125 "EcrioCPIMParseMessage.c" */
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
/* #line 221 "EcrioCPIMParseMessage.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
	}
	goto st0;
/* #line 147 "EcrioCPIMParseMessage.c" */
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	if ( (*p) == 10 )
		goto st122;
	goto tr0;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	goto tr137;
tr137:
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st123;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
/* #line 173 "EcrioCPIMParseMessage.c" */
	goto st123;
tr2:
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
/* #line 185 "EcrioCPIMParseMessage.c" */
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
		case 60: goto tr31;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr31;
		} else if ( (*p) >= 33 )
			goto tr31;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
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
		case 60: goto tr31;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr31;
		} else if ( (*p) >= 33 )
			goto tr31;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
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
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
/* #line 441 "EcrioCPIMParseMessage.c" */
	switch( (*p) ) {
		case 13: goto tr34;
		case 37: goto st32;
		case 60: goto st32;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st32;
		} else if ( (*p) >= 33 )
			goto st32;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
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
/* #line 183 "EcrioCPIMParseMessage.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentTransferEncoding,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st33;
tr62:
/* #line 173 "EcrioCPIMParseMessage.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentDisposition,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st33;
tr70:
/* #line 178 "EcrioCPIMParseMessage.rl" */
	{
		uCPIMError = ec_cpim_StringCopy(&c->strings,
			(void**)&pContentId,
			(u_char*)tag_start,
			(p - tag_start));
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			goto END;
		}
	}
	goto st33;
tr82:
/* #line 161 "EcrioCPIMParseMessage.rl" */
	{
		pal_MemorySet(cTemp, 0, 8);
		if (NULL == pal_StringNCopy(cTemp, 8, (u_char*)tag_start, (p - tag_start)))
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tMemory copy error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		uCntLen = pal_StringToNum(cTemp, NULL);
	}
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
/* #line 500 "EcrioCPIMParseMessage.c" */
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
		case 73: goto st57;
		case 76: goto st64;
		case 84: goto st75;
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
		case 60: goto tr59;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr59;
		} else if ( (*p) >= 33 )
			goto tr59;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
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
		case 60: goto tr59;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr59;
		} else if ( (*p) >= 33 )
			goto tr59;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
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
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st56;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
/* #line 716 "EcrioCPIMParseMessage.c" */
	switch( (*p) ) {
		case 13: goto tr62;
		case 37: goto st56;
		case 60: goto st56;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st56;
		} else if ( (*p) >= 33 )
			goto st56;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto st56;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st56;
		} else
			goto st56;
	} else
		goto st56;
	goto tr0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 68 )
		goto st58;
	goto tr0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	switch( (*p) ) {
		case 9: goto st58;
		case 32: goto st58;
		case 58: goto st59;
	}
	goto tr0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 13: goto st60;
		case 32: goto st59;
		case 37: goto tr67;
		case 60: goto tr67;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr67;
		} else if ( (*p) >= 33 )
			goto tr67;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto tr67;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr67;
		} else
			goto tr67;
	} else
		goto tr67;
	goto tr0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 10 )
		goto st61;
	goto tr0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( (*p) == 32 )
		goto st62;
	goto tr0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	switch( (*p) ) {
		case 32: goto st62;
		case 37: goto tr67;
		case 60: goto tr67;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr67;
		} else if ( (*p) >= 33 )
			goto tr67;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto tr67;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr67;
		} else
			goto tr67;
	} else
		goto tr67;
	goto tr0;
tr67:
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st63;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
/* #line 836 "EcrioCPIMParseMessage.c" */
	switch( (*p) ) {
		case 13: goto tr70;
		case 37: goto st63;
		case 60: goto st63;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st63;
		} else if ( (*p) >= 33 )
			goto st63;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto st63;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st63;
		} else
			goto st63;
	} else
		goto st63;
	goto tr0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 101 )
		goto st65;
	goto tr0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( (*p) == 110 )
		goto st66;
	goto tr0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( (*p) == 103 )
		goto st67;
	goto tr0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( (*p) == 116 )
		goto st68;
	goto tr0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( (*p) == 104 )
		goto st69;
	goto tr0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	switch( (*p) ) {
		case 9: goto st69;
		case 32: goto st69;
		case 58: goto st70;
	}
	goto tr0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	switch( (*p) ) {
		case 13: goto st71;
		case 32: goto st70;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr79;
	goto tr0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 10 )
		goto st72;
	goto tr0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 32 )
		goto st73;
	goto tr0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 32 )
		goto st73;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr79;
	goto tr0;
tr79:
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
/* #line 949 "EcrioCPIMParseMessage.c" */
	if ( (*p) == 13 )
		goto tr82;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st74;
	goto tr0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	switch( (*p) ) {
		case 114: goto st76;
		case 121: goto st91;
	}
	goto tr0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	if ( (*p) == 97 )
		goto st77;
	goto tr0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( (*p) == 110 )
		goto st78;
	goto tr0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( (*p) == 115 )
		goto st79;
	goto tr0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( (*p) == 102 )
		goto st80;
	goto tr0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	if ( (*p) == 101 )
		goto st81;
	goto tr0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	if ( (*p) == 114 )
		goto st82;
	goto tr0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( (*p) == 45 )
		goto st83;
	goto tr0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( (*p) == 69 )
		goto st84;
	goto tr0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	if ( (*p) == 110 )
		goto st85;
	goto tr0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( (*p) == 99 )
		goto st86;
	goto tr0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	if ( (*p) == 111 )
		goto st87;
	goto tr0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	if ( (*p) == 100 )
		goto st88;
	goto tr0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	if ( (*p) == 105 )
		goto st89;
	goto tr0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( (*p) == 110 )
		goto st90;
	goto tr0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	if ( (*p) == 103 )
		goto st27;
	goto tr0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( (*p) == 112 )
		goto st92;
	goto tr0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	if ( (*p) == 101 )
		goto st93;
	goto tr0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	switch( (*p) ) {
		case 9: goto st93;
		case 32: goto st93;
		case 58: goto st94;
	}
	goto tr0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	switch( (*p) ) {
		case 13: goto st95;
		case 32: goto st94;
		case 37: goto tr104;
		case 60: goto tr104;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr104;
		} else if ( (*p) >= 33 )
			goto tr104;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto tr104;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr104;
		} else
			goto tr104;
	} else
		goto tr104;
	goto tr0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( (*p) == 10 )
		goto st96;
	goto tr0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( (*p) == 32 )
		goto st97;
	goto tr0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	switch( (*p) ) {
		case 32: goto st97;
		case 37: goto tr104;
		case 60: goto tr104;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr104;
		} else if ( (*p) >= 33 )
			goto tr104;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto tr104;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr104;
		} else
			goto tr104;
	} else
		goto tr104;
	goto tr0;
tr104:
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st98;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
/* #line 1172 "EcrioCPIMParseMessage.c" */
	switch( (*p) ) {
		case 13: goto tr107;
		case 32: goto tr108;
		case 37: goto st98;
		case 59: goto tr110;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st98;
		} else if ( (*p) >= 33 )
			goto st98;
	} else if ( (*p) > 60 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto st98;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st98;
		} else
			goto st98;
	} else
		goto st98;
	goto tr0;
tr107:
/* #line 80 "EcrioCPIMParseMessage.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
				pal_StringLength((u_char*)"text/plain")) == 0)
		{
			eConType = CPIMContentType_Text;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_IMDN_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_IMDN_STRING)) == 0)
		{
			eConType = CPIMContentType_IMDN;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING)) == 0)
		{
			eConType = CPIMContentType_FileTransferOverHTTP;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING)) == 0)
		{
			eConType = CPIMContentType_PushLocation;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING)) == 0)
		{
			eConType = CPIMContentType_MessageRevoke;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
		{
			eConType = CPIMContentType_PrivacyManagement;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_LINK_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_LINK_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_LinkReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SPAM_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_SpamReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_RICHCARD_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_RICHCARD_STRING)) == 0)
		{
			eConType = CPIMContentType_RichCard;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestedChipList;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestionResponse;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
		{
			eConType = CPIMContentType_SharedClientData;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING)) == 0)
		{
			eConType = CPIMContentType_CpmGroupData;
		}
		else
		{
			eConType = CPIMContentType_Specified;
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pContentType,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st99;
tr130:
/* #line 76 "EcrioCPIMParseMessage.rl" */
	{
	}
	goto st99;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
/* #line 1288 "EcrioCPIMParseMessage.c" */
	if ( (*p) == 10 )
		goto st100;
	goto tr0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	switch( (*p) ) {
		case 13: goto st2;
		case 32: goto st101;
		case 67: goto tr2;
	}
	goto tr0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	switch( (*p) ) {
		case 32: goto st101;
		case 59: goto st102;
	}
	goto tr0;
tr110:
/* #line 80 "EcrioCPIMParseMessage.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
				pal_StringLength((u_char*)"text/plain")) == 0)
		{
			eConType = CPIMContentType_Text;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_IMDN_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_IMDN_STRING)) == 0)
		{
			eConType = CPIMContentType_IMDN;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING)) == 0)
		{
			eConType = CPIMContentType_FileTransferOverHTTP;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING)) == 0)
		{
			eConType = CPIMContentType_PushLocation;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING)) == 0)
		{
			eConType = CPIMContentType_MessageRevoke;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
		{
			eConType = CPIMContentType_PrivacyManagement;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_LINK_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_LINK_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_LinkReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SPAM_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_SpamReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_RICHCARD_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_RICHCARD_STRING)) == 0)
		{
			eConType = CPIMContentType_RichCard;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestedChipList;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestionResponse;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
		{
			eConType = CPIMContentType_SharedClientData;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING)) == 0)
		{
			eConType = CPIMContentType_CpmGroupData;
		}
		else
		{
			eConType = CPIMContentType_Specified;
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pContentType,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st102;
tr133:
/* #line 76 "EcrioCPIMParseMessage.rl" */
	{
	}
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
/* #line 1402 "EcrioCPIMParseMessage.c" */
	switch( (*p) ) {
		case 13: goto st103;
		case 32: goto st102;
		case 37: goto st106;
		case 60: goto st106;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st106;
		} else if ( (*p) >= 33 )
			goto st106;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto st106;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st106;
		} else
			goto st106;
	} else
		goto st106;
	goto tr0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( (*p) == 10 )
		goto st104;
	goto tr0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	if ( (*p) == 32 )
		goto st105;
	goto tr0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	switch( (*p) ) {
		case 32: goto st105;
		case 37: goto st106;
		case 60: goto st106;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st106;
		} else if ( (*p) >= 33 )
			goto st106;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto st106;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st106;
		} else
			goto st106;
	} else
		goto st106;
	goto tr0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	switch( (*p) ) {
		case 13: goto st107;
		case 32: goto st121;
		case 37: goto st106;
		case 61: goto st110;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st106;
		} else if ( (*p) >= 33 )
			goto st106;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 60 <= (*p) && (*p) <= 93 )
				goto st106;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st106;
		} else
			goto st106;
	} else
		goto st106;
	goto tr0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( (*p) == 10 )
		goto st108;
	goto tr0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( (*p) == 32 )
		goto st109;
	goto tr0;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	switch( (*p) ) {
		case 32: goto st109;
		case 61: goto st110;
	}
	goto tr0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	switch( (*p) ) {
		case 13: goto st111;
		case 32: goto st110;
		case 37: goto tr124;
		case 60: goto tr124;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr124;
		} else if ( (*p) >= 33 )
			goto tr124;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto tr124;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr124;
		} else
			goto tr124;
	} else
		goto tr124;
	goto tr0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	if ( (*p) == 10 )
		goto st112;
	goto tr0;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	if ( (*p) == 32 )
		goto st113;
	goto tr0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	switch( (*p) ) {
		case 13: goto st114;
		case 32: goto st113;
		case 37: goto tr124;
		case 60: goto tr124;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr124;
		} else if ( (*p) >= 33 )
			goto tr124;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto tr124;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr124;
		} else
			goto tr124;
	} else
		goto tr124;
	goto tr0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( (*p) == 10 )
		goto st115;
	goto tr0;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	if ( (*p) == 32 )
		goto st116;
	goto tr0;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	switch( (*p) ) {
		case 32: goto st116;
		case 37: goto tr124;
		case 60: goto tr124;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto tr124;
		} else if ( (*p) >= 33 )
			goto tr124;
	} else if ( (*p) > 58 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto tr124;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto tr124;
		} else
			goto tr124;
	} else
		goto tr124;
	goto tr0;
tr124:
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
	goto st117;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
/* #line 1640 "EcrioCPIMParseMessage.c" */
	switch( (*p) ) {
		case 13: goto tr130;
		case 32: goto tr131;
		case 37: goto st117;
		case 59: goto tr133;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 34 ) {
			if ( 39 <= (*p) && (*p) <= 43 )
				goto st117;
		} else if ( (*p) >= 33 )
			goto st117;
	} else if ( (*p) > 60 ) {
		if ( (*p) < 95 ) {
			if ( 62 <= (*p) && (*p) <= 93 )
				goto st117;
		} else if ( (*p) > 123 ) {
			if ( 125 <= (*p) && (*p) <= 126 )
				goto st117;
		} else
			goto st117;
	} else
		goto st117;
	goto tr0;
tr108:
/* #line 80 "EcrioCPIMParseMessage.rl" */
	{
		if (pal_StringNCompare((u_char*)tag_start, (u_char*)"text/plain",
				pal_StringLength((u_char*)"text/plain")) == 0)
		{
			eConType = CPIMContentType_Text;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_IMDN_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_IMDN_STRING)) == 0)
		{
			eConType = CPIMContentType_IMDN;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_FILE_TRANSFER_HTTP_STRING)) == 0)
		{
			eConType = CPIMContentType_FileTransferOverHTTP;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PUSH_LOCATION_STRING)) == 0)
		{
			eConType = CPIMContentType_PushLocation;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_MESSAGE_REVOKE_STRING)) == 0)
		{
			eConType = CPIMContentType_MessageRevoke;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_PRIVACY_MANAGEMENT_STRING)) == 0)
		{
			eConType = CPIMContentType_PrivacyManagement;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_LINK_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_LINK_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_LinkReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SPAM_REPORT_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SPAM_REPORT_STRING)) == 0)
		{
			eConType = CPIMContentType_SpamReport;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_RICHCARD_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_RICHCARD_STRING)) == 0)
		{
			eConType = CPIMContentType_RichCard;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_CHIPLIST_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestedChipList;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SUGGESTED_RESPONSE_STRING)) == 0)
		{
			eConType = CPIMContentType_SuggestionResponse;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_SHARED_CLIENT_DATA_STRING)) == 0)
		{
			eConType = CPIMContentType_SharedClientData;
		}
		else if (pal_StringNCompare((u_char*)tag_start, CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING,
					pal_StringLength(CPIM_CONTENT_TYPE_CPM_GROUPDATA_STRING)) == 0)
		{
			eConType = CPIMContentType_CpmGroupData;
		}
		else
		{
			eConType = CPIMContentType_Specified;
			uCPIMError = ec_cpim_StringCopy(&c->strings,
				(void**)&pContentType,
				(u_char*)tag_start,
				(p - tag_start));
			if (uCPIMError != ECRIO_CPIM_NO_ERROR)
			{
				goto END;
			}
		}
	}
	goto st118;
tr131:
/* #line 76 "EcrioCPIMParseMessage.rl" */
	{
	}
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
/* #line 1756 "EcrioCPIMParseMessage.c" */
	switch( (*p) ) {
		case 13: goto st119;
		case 32: goto st118;
		case 59: goto st102;
	}
	goto tr0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( (*p) == 10 )
		goto st120;
	goto tr0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	if ( (*p) == 32 )
		goto st101;
	goto tr0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	switch( (*p) ) {
		case 13: goto st107;
		case 32: goto st121;
		case 61: goto st110;
	}
	goto tr0;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
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

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 123: 
/* #line 188 "EcrioCPIMParseMessage.rl" */
	{
		if (uCntLen == 0 || uCntLen > (p - tag_start))
		{
			uCntLen = p - tag_start;
		}

		c->buff[uIndex].uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[uIndex], uCntLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->buff[uIndex].pData, 0, c->buff[uIndex].uContainerSize);
		pal_MemoryCopy(c->buff[uIndex].pData, c->buff[uIndex].uContainerSize, (u_char*)tag_start, uCntLen);
		c->buff[uIndex].uSize = uCntLen;

		pStruct->uNumOfBuffers++;
		pStruct->buff[uIndex].eContentType = eConType;
		if (eConType == CPIMContentType_Specified)
		{
			pStruct->buff[uIndex].pContentType = pContentType;
		}
		else
		{
			pStruct->buff[uIndex].pContentType = NULL;
		}
		pStruct->buff[uIndex].pMsgBody = c->buff[uIndex].pData;
		pStruct->buff[uIndex].uMsgLen = c->buff[uIndex].uSize;
		if (pContentId != NULL)
		{
			pStruct->buff[uIndex].pContentId = pContentId;
		}
		else
		{
			pStruct->buff[uIndex].pContentId = NULL;
		}
		if (pContentDisposition != NULL)
		{
			pStruct->buff[uIndex].pContentDisposition = pContentDisposition;
		}
		else
		{
			pStruct->buff[uIndex].pContentDisposition = NULL;
		}
		if (pContentTransferEncoding != NULL)
		{
			pStruct->buff[uIndex].pContentTransferEncoding = pContentTransferEncoding;
		}
		else
		{
			pStruct->buff[uIndex].pContentTransferEncoding = NULL;
		}
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
/* #line 221 "EcrioCPIMParseMessage.rl" */
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uCPIMError = ECRIO_CPIM_PARSING_INSUFFICIENT_ERROR;
	}
	break;
	case 122: 
/* #line 71 "EcrioCPIMParseMessage.rl" */
	{
		tag_start = p;
	}
/* #line 188 "EcrioCPIMParseMessage.rl" */
	{
		if (uCntLen == 0 || uCntLen > (p - tag_start))
		{
			uCntLen = p - tag_start;
		}

		c->buff[uIndex].uSize = 0;
		uCPIMError = ec_cpim_MaintenanceBuffer(&c->buff[uIndex], uCntLen);
		if (uCPIMError != ECRIO_CPIM_NO_ERROR)
		{
			CPIMLOGE(c->logHandle, KLogTypeGeneral, "%s:%u\tThe working buffer maintenance error.", __FUNCTION__, __LINE__);
			uCPIMError = ECRIO_CPIM_MEMORY_ALLOCATION_ERROR;
			goto END;
		}
		pal_MemorySet(c->buff[uIndex].pData, 0, c->buff[uIndex].uContainerSize);
		pal_MemoryCopy(c->buff[uIndex].pData, c->buff[uIndex].uContainerSize, (u_char*)tag_start, uCntLen);
		c->buff[uIndex].uSize = uCntLen;

		pStruct->uNumOfBuffers++;
		pStruct->buff[uIndex].eContentType = eConType;
		if (eConType == CPIMContentType_Specified)
		{
			pStruct->buff[uIndex].pContentType = pContentType;
		}
		else
		{
			pStruct->buff[uIndex].pContentType = NULL;
		}
		pStruct->buff[uIndex].pMsgBody = c->buff[uIndex].pData;
		pStruct->buff[uIndex].uMsgLen = c->buff[uIndex].uSize;
		if (pContentId != NULL)
		{
			pStruct->buff[uIndex].pContentId = pContentId;
		}
		else
		{
			pStruct->buff[uIndex].pContentId = NULL;
		}
		if (pContentDisposition != NULL)
		{
			pStruct->buff[uIndex].pContentDisposition = pContentDisposition;
		}
		else
		{
			pStruct->buff[uIndex].pContentDisposition = NULL;
		}
		if (pContentTransferEncoding != NULL)
		{
			pStruct->buff[uIndex].pContentTransferEncoding = pContentTransferEncoding;
		}
		else
		{
			pStruct->buff[uIndex].pContentTransferEncoding = NULL;
		}
	}
	break;
/* #line 2115 "EcrioCPIMParseMessage.c" */
	}
	}

	_out: {}
	}

/* #line 301 "EcrioCPIMParseMessage.rl" */

END:

	CPIMLOGI(c->logHandle, KLogTypeFuncExit, "%s:%u", __FUNCTION__, __LINE__);

	return uCPIMError;
}
