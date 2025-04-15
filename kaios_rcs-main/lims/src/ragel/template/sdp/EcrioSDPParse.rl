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
* @file EcrioSDPParse.c
* @brief Implementation of SDP module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioSDP.h"
#include "EcrioSDPInternal.h"

%%{
	machine ec_sdp_Parser;

	CR = "\r";
	LF = "\n";
	SP = " ";
	CRLF = CR LF;

	action tag
	{
		tag_start = p;
	}

	action ParseHeaderA
	{
		if (bIsMaxMedia != Enum_TRUE)
		{
			/** Check for underflow of uNumOfMedia. Here, we assume that uNumOfMedia is
			 *  greater than or equal to 1 in order to handle the media-level attributes.
			 */
			if (Enum_FALSE == pal_UtilityArithmeticUnderflowDetected(pSdpStruct->uNumOfMedia, 1))
			{
				uError = ec_sdp_ParseHeaderA(&pSdpStruct->strings,
											 &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1],
											 (u_char*)tag_start,
											 (p - tag_start));
				if (uError != ECRIO_SDP_NO_ERROR)
				{
					goto END;
				}
			}
		}
	}

	action ParseHeaderB
	{
		if (pSdpStruct->uNumOfMedia == 0)
		{
			pBandwidth = &pSdpStruct->bandwidth;
		}
		else if (bIsMaxMedia != Enum_TRUE)
		{
			pBandwidth = &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1].bandwidth;
		}
		else
		{
			goto END;
		}

		uError = ec_sdp_ParseHeaderB(pBandwidth,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseHeaderC
	{
		if (pSdpStruct->uNumOfMedia == 0)
		{
			pConn = &pSdpStruct->conn;
		}
		else if (bIsMaxMedia != Enum_TRUE)
		{
			pConn = &pSdpStruct->stream[pSdpStruct->uNumOfMedia - 1].conn;
		}
		else
		{
			goto END;
		}

		uError = ec_sdp_ParseHeaderC(&pSdpStruct->strings,
									 pConn,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseHeaderM
	{
		if (pSdpStruct->uNumOfMedia == MAX_MEDIA)
		{
			/** media slot is full, then later media line do not parse. */
			bIsMaxMedia = Enum_TRUE;
			goto END;
		}

		uError = ec_sdp_ParseHeaderM(&pSdpStruct->stream[pSdpStruct->uNumOfMedia],
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
		pSdpStruct->uNumOfMedia++;
	}

	action ParseHeaderO
	{
		uError = ec_sdp_ParseHeaderO(&pSdpStruct->strings,
									 &pSdpStruct->origin,
									 (u_char*)tag_start,
									 (p - tag_start));
		if (uError != ECRIO_SDP_NO_ERROR)
		{
			goto END;
		}
	}

	action ParseHeaderS
	{
		/** No check a session name */
		uError = ECRIO_SDP_NO_ERROR;
	}

	action ParseHeaderT
	{
		/** No check a start/end time */
		uError = ECRIO_SDP_NO_ERROR;
	}

	action ParseHeaderV
	{
		/** No check a version number */
		uError = ECRIO_SDP_NO_ERROR;
	}

	action ParseHeaderOther
	{
		/** No action when unsupported header was given */
		uError = ECRIO_SDP_NO_ERROR;
	}

	action error
	{
		/** This is a general error, but usually no CRLF at the end of line. */
		uError = ECRIO_SDP_PARSING_INSUFFICIENT_ERROR;
	}

	A = "a"i SP* "=" SP*<: any* %ParseHeaderA :>(CRLF | LF);
	B = "b"i SP* "=" SP*<: any* %ParseHeaderB :>(CRLF | LF);
	C = "c"i SP* "=" SP*<: any* %ParseHeaderC :>(CRLF | LF);
	M = "m"i SP* "=" SP*<: any* %ParseHeaderM :>(CRLF | LF);
	O = "o"i SP* "=" SP*<: any* %ParseHeaderO :>(CRLF | LF);
	S = "s"i SP* "=" SP*<: any* %ParseHeaderS :>(CRLF | LF);
	T = "t"i SP* "=" SP*<: any* %ParseHeaderT :>(CRLF | LF);
	V = "v"i SP* "=" SP*<: any* %ParseHeaderV :>(CRLF | LF);
	Other = alpha SP* "=" SP*<: any* %ParseHeaderOther :>(CRLF | LF);

	header = (A | B | C | M | O | S | T | V)>tag >1 | (Other>tag) >0;
	SDPMessage = header*;
	main := SDPMessage @!error;
	
	write data;
}%%

/**
 * This is used to parse a SDP message and fill up the SDP structure.
 * 
 * @param[out]	pSdpStruct		Pointer to the SDP structure. Must be non-NULL.
 * @param[in]	pData			Pointer to the SDP message. Must be non-NULL.
 * @param[in]	uSize			The size of SDP buffer.
 * @return error_none if successful, otherwise a specific error.
 */
u_int32 EcrioSDPParse
(
	EcrioSDPSessionStruct *pSdpStruct,
	u_char *pData,
	u_int32 uSize
)
{
	u_int32	uError = ECRIO_SDP_NO_ERROR;
	s_int32	cs;
	const char	*p;
	const char	*pe;
	const char	*eof;
	const char	*tag_start = NULL;
	EcrioSDPBandwidthStruct	*pBandwidth = NULL;
	EcrioSDPConnectionInfomationStruct	*pConn = NULL;
	BoolEnum	bIsMaxMedia = Enum_FALSE;

	/** Parameter check */
	if (pSdpStruct == NULL || pData == NULL || uSize == 0)
	{
		return ECRIO_SDP_INSUFFICIENT_DATA_ERROR;
	}

	pal_MemorySet(&pSdpStruct->origin, 0, sizeof(EcrioSDPOriginStruct));
	pal_MemorySet(&pSdpStruct->conn, 0, sizeof(EcrioSDPConnectionInfomationStruct));
	pal_MemorySet(&pSdpStruct->bandwidth, 0, sizeof(EcrioSDPBandwidthStruct));
	pSdpStruct->uNumOfMedia = 0;
	pal_MemorySet(&pSdpStruct->stream, 0, sizeof(EcrioSDPStreamStruct) * MAX_MEDIA);

	p = (char*)pData;
	pe = (char*)pData + uSize;
	eof = pe;

	/** Parsing [<type>=<value>] to using Ragel*/
	%% write init;
	%% write exec;

END:
	return uError;
}
