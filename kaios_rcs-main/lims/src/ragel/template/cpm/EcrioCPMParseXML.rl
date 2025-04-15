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
 * @file EcrioCPMParseXML.c
 * @brief Implementation of the CPM module parsing functionality.
*/

#include "EcrioPAL.h"
#include "EcrioCPM.h"
#include "EcrioCPMInternal.h"

%%{
	machine ec_cpm_XMLParser;

	CRLF = "\r\n";
	DQUOTE = "\"";
	SP = " ";

	alphanum = [a-zA-Z0-9];

	LWS    = ((" "* CRLF)? " "+);
	SWS    = (LWS?);

	token  = (alphanum | "-" | "=" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" )+;
	word   = (alphanum | "-" | "=" | "." | "!" | "%" | "*" | "_" | "+" | "`" | "'" | "~" | "(" | ")" | "<" | ">" | ":" | "@" | "\\" | DQUOTE | "/" | "[" | "]" | "?" | "{" | "}")+;

	qdtext = (LWS | 0x21 | 0x23..0x5B | 0x5D..0x7E);
	quoted_pair = ("\\" (0x00..0x09 | 0x0B..0x0C | 0x0E..0x7F));
	quoted_string = (SWS DQUOTE (qdtext | quoted_pair)* DQUOTE);

	sub_colon = ((" " | "\t")* ":");

	COMMA  = (SWS "," SWS);
	RAQUOT = (">" SWS);
	LAQUOT = (SWS "<");
	EQUAL  = (SWS "=" SWS);
	SEMI   = (SWS ";" SWS);
	HCOLON = ((" " | "\t")* ":" SWS);

	action tag
	{
		tag_start = p;
	}

	action startValue
	{
		value_start = p;
		bIsValue = Enum_TRUE;
	}

	action elementName
	{
		if (bIsValue == Enum_TRUE)
		{
			bIsValue = Enum_FALSE;
		}

		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct), (void **)&pStruct);
		if (NULL == pStruct)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pal_MemorySet((void*)pStruct, 0, sizeof(EcrioCPMParsedXMLStruct));
		pStruct->parent = (void*)pCurr;
		pCurr = pStruct;

		pStruct->pElementName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action elementValue
	{
		if (bIsValue == Enum_TRUE)
		{
			pStruct->pElementValue = pal_StringCreate((u_char*)value_start, p - value_start - 2);
		}
		bIsValue = Enum_FALSE;
	}

	action attributeName
	{
		pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct), (void **)&pAttr);
		if (NULL == pAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}

		if (pStruct->uNumOfAttr == 0)
		{
			pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLAttrStruct*), (void **)&pStruct->ppAttr);
		}
		else
		{
			/* Check arithmetic overflow */
			if (pal_UtilityArithmeticOverflowDetected(pStruct->uNumOfAttr, 1) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}

			/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
			if (pal_UtilityDataOverflowDetected((pStruct->uNumOfAttr + 1), sizeof(EcrioCPMParsedXMLAttrStruct*)) == Enum_TRUE)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;
				goto END;
			}
			pal_MemoryReallocate((u_int32)((pStruct->uNumOfAttr + 1) * sizeof(EcrioCPMParsedXMLAttrStruct*)), (void **)&pStruct->ppAttr);
		}
		if (NULL == pStruct->ppAttr)
		{
			uCPMError = ECRIO_CPM_MEMORY_ERROR;

			CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
				__FUNCTION__, __LINE__, uCPMError);
			goto END;
		}
		pStruct->ppAttr[pStruct->uNumOfAttr] = pAttr;
		pStruct->uNumOfAttr++;

		pAttr->pAttrName = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action attributeValue
	{
		pAttr->pAttrValue = pal_StringCreate((u_char*)tag_start, p - tag_start);
	}

	action finalize
	{
		if (pCurr->parent == ppParsedXml)
		{
			*ppParsedXml = pCurr;
		}
		else
		{
			pStruct = pCurr;
			pCurr = (EcrioCPMParsedXMLStruct*)pCurr->parent;

			if (pCurr->uNumOfChild == 0)
			{
				pal_MemoryAllocate((u_int32)sizeof(EcrioCPMParsedXMLStruct*), (void **)&pCurr->ppChild);
			}
			else
			{
				/* Check arithmetic overflow */
				if (pal_UtilityArithmeticOverflowDetected(pCurr->uNumOfChild, 1) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}

				/* Check for memory overflow - exclude if memory to be allocated exceeds limit of u_int32 */
				if (pal_UtilityDataOverflowDetected((pCurr->uNumOfChild + 1), sizeof(EcrioCPMParsedXMLStruct*)) == Enum_TRUE)
				{
					uCPMError = ECRIO_CPM_MEMORY_ERROR;
					goto END;
				}
				pal_MemoryReallocate((u_int32)((pCurr->uNumOfChild + 1) * sizeof(EcrioCPMParsedXMLStruct*)), (void **)&pCurr->ppChild);
			}
			if (NULL == pCurr->ppChild)
			{
				uCPMError = ECRIO_CPM_MEMORY_ERROR;

				CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpal_MemoryAllocate() uError=%u",
					__FUNCTION__, __LINE__, uCPMError);
				goto END;
			}
			pCurr->ppChild[pCurr->uNumOfChild] = pStruct;
			pCurr->uNumOfChild++;
		}
	}

	action error
	{
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tError: p = 0x%x, number = %d, pe = 0x%x", __FUNCTION__, __LINE__, p, p - pData, pe);
		CPMLOGE(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tValue: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x |%2.2x| %2.2x %2.2x %2.2x", __FUNCTION__, __LINE__,
			*(p-8), *(p-7), *(p-6), *(p-5), *(p-4), *(p-3), *(p-2), *(p-1), *(p), *(p+1), *(p+2), *(p+3));
		uCPMError = ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	Extender = 0xB7;
	Digit = (0x30..0x39);
	Char = 0x9 | 0xA | 0xD | (0x20..0x7F) | (0x80..0xFF);
	BaseChar = (0x41..0x5A) | (0x61..0x7A) | (0xC0..0xD6) | (0xD8..0xF6) | (0xF8..0xFF);
	Letter = BaseChar;
	NameChar = Letter | Digit | '.' | '-' | '_' | ':' | Extender;
	S = (0x20 | 0x9 | 0xD | 0xA)+;
	PubidChar = 0x20 | 0xD | 0xA | [a-zA-Z0-9] | [\-'()+,./:=?;!*#@$_%];
	PubidLiteral = '"' PubidChar* '"' | "'" (PubidChar - "'")* "'";
	Name = (Letter | '_' | ':') (NameChar)*;
	Comment = '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->';
	CharData_Old = [^<&]* - ([^<&]* ']]>' [^<&]*);
	CharData = [^<&]+ -- ']]>';
	SystemLiteral = ('"' [^"]* '"') | ("'" [^']* "'");
	Eq = S? '=' S?;
	VersionNum = ([a-zA-Z0-9_.:] | '-')+;
	VersionInfo = S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"');
	ExternalID = 'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral;
	PublicID = 'PUBLIC' S PubidLiteral;
	NotationDecl = '<!NOTATION' S Name S (ExternalID |  PublicID) S? '>';
	EncName = [A-Za-z] ([A-Za-z0-9._] | '-')*;
	EncodingDecl = S 'encoding' Eq ('"' EncName  '"' |  "'" EncName "'" );
	NDataDecl = S 'NDATA' S Name;
	PEReference = '%' Name ';';
	EntityRef = '&' Name ';';
	CharRef = '&#' [0-9]+ ';' | '&0x' [0-9a-fA-F]+ ';';
	Reference = EntityRef | CharRef;
	EntityValue = '"' ([^%&"] | PEReference | Reference)* '"' |  "'" ([^%&'] | PEReference | Reference)* "'";
	PEDef = EntityValue | ExternalID;
	EntityDef = EntityValue | (ExternalID NDataDecl?);
	PEDecl = '<!ENTITY' S '%' S Name S PEDef S? '>';
	GEDecl = '<!ENTITY' S Name S EntityDef S? '>';
	EntityDecl = GEDecl | PEDecl;
	Mixed = '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*' | '(' S? '#PCDATA' S? ')';
	alt = '?' | '*' | '+';
	children = '(' S? 
				( ( Name alt? )  | 
				'(' | 
				( ')' alt? ) | 
				[,|] |
				S )
	 			')' alt?;
	contentspec = 'EMPTY' | 'ANY' | Mixed | children;
	elementdecl = '<!ELEMENT' S Name S contentspec S? '>';
	AttValue = '"' ([^<&"] | Reference)* >tag %attributeValue '"' |  "'" ([^<&'] | Reference)* >tag %attributeValue "'";
	Attribute = Name >tag %attributeName Eq AttValue;
	Nmtoken = (NameChar)+;
	Enumeration = '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')';
	NotationType = 'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')';
	EnumeratedType = NotationType | Enumeration;
	TokenizedType = 'ID' | 'IDREF' | 'IDREFS' | 'ENTITY' | 'ENTITIES' | 'NMTOKEN' | 'NMTOKENS';
	StringType = 'CDATA';
	AttType = StringType | TokenizedType | EnumeratedType;
	DefaultDecl = '#REQUIRED' | '#IMPLIED' | (('#FIXED' S)? AttValue);
	AttDef = S Name S AttType S DefaultDecl;
	AttlistDecl = '<!ATTLIST' S Name AttDef* S? '>';
	EmptyElemTag = '<' Name >tag %elementName (S Attribute)* S? '/>' %finalize;
	ETag = '</' %elementValue Name S? '>' %finalize;
	PITarget_Old = Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'));
	PITarget = Name -- "xml"i;
	PI = '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>';
	markupdecl = elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment;
	doctypedecl = '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | PEReference | S)* ']' S?)? '>';
	STag = '<' Name >tag  %elementName (S Attribute)* S? '>' %startValue;
	CDStart = '<![CDATA[';
	CDEnd = ']]>';
	CData = (Char* -- CDEnd);
	CDSect = CDStart CData CDEnd;
	SDDecl = S 'standalone' Eq (("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"'));
	Misc = Comment | PI |  S;
	XMLDecl = '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>';
	prolog = XMLDecl? Misc* (doctypedecl Misc*)?;
	content = (EmptyElemTag | STag | ETag | CharData | Reference | CDSect | PI | Comment)*;
	document = prolog ( EmptyElemTag | ( STag content ETag ) ) Misc*;
	main := document @!error;

	write data;
}%%

u_int32 ec_CPM_ParseXmlDocument
(
	EcrioCPMContextStruct *pContext,
	EcrioCPMParsedXMLStruct **ppParsedXml,
	char *pData,
	u_int32 uLen
)
{
	u_int32 uCPMError = ECRIO_CPM_NO_ERROR;
	EcrioCPMParsedXMLStruct *pCurr = NULL;
	EcrioCPMParsedXMLStruct *pStruct = NULL;
	EcrioCPMParsedXMLAttrStruct *pAttr = NULL;
	s_int32 cs = 1;
	const char *p;
	const char *pe;
	const char *eof;
	const char *tag_start = NULL;
	const char *value_start = NULL;
	BoolEnum bIsValue = Enum_FALSE;

	if (pContext == NULL || ppParsedXml == NULL || pData == NULL || uLen == 0)
	{
		return ECRIO_CPM_INSUFFICIENT_DATA_ERROR;
	}

	CPMLOGI(pContext->hLogHandle, KLogTypeFuncEntry, "%s:%u", __FUNCTION__, __LINE__);
	CPMLOGI(pContext->hLogHandle, KLogTypeGeneral, "%s:%u\tpData = %x, uLen = %d", __FUNCTION__, __LINE__, pData, uLen);

	pCurr = (void*)ppParsedXml;

	p = pData;
	pe = pData + uLen;
	eof = pe;

	%% write init;
	%% write exec;

END:
	CPMLOGI(pContext->hLogHandle, KLogTypeFuncExit, "%s:%u\tuCPMError=%u", __FUNCTION__, __LINE__, uCPMError);

	return uCPMError;
}

void ec_CPM_ReleaseParsedXmlStruct
(
	EcrioCPMParsedXMLStruct **ppParsedXml,
	BoolEnum release
)
{
	u_int32 i;
	EcrioCPMParsedXMLStruct *pParsedXml;

	if (ppParsedXml == NULL || *ppParsedXml == NULL)
	{
		return;
	}

	pParsedXml = *ppParsedXml;

	pal_MemoryFree((void**)&pParsedXml->pElementName);
	pal_MemoryFree((void**)&pParsedXml->pElementValue);

	for (i = 0; i < pParsedXml->uNumOfAttr; i++)
	{
		pal_MemoryFree((void**)&pParsedXml->ppAttr[i]->pAttrName);
		pal_MemoryFree((void**)&pParsedXml->ppAttr[i]->pAttrValue);
		pal_MemoryFree((void**)&pParsedXml->ppAttr[i]);
	}
	pal_MemoryFree((void**)&pParsedXml->ppAttr);

	for (i = 0; i < pParsedXml->uNumOfChild; i++)
	{
		ec_CPM_ReleaseParsedXmlStruct((EcrioCPMParsedXMLStruct**)&pParsedXml->ppChild[i], Enum_TRUE);
	}
	pal_MemoryFree((void**)&pParsedXml->ppChild);

	if (release == Enum_TRUE)
	{
		pal_MemoryFree((void**)ppParsedXml);
	}
}
