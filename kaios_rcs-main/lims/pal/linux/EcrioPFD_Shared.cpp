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
 * @file EcrioPFD_Shared.c
 * @brief This is the implementation of shared code for modules implemented
 * on the Android platform.
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

/* Network interface-related includes */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

/* Include this module's PFD header file. */
#include "EcrioPFD_Shared.h"
#if 0
/* Include the Memory module PFD header file since it is a shared dependency. */
#include "EcrioPFD_Memory.h"

/**
 * Implementation of the GetTickCount() function, which should return the
 * number of milliseconds since a reference point in the past. Usually, the
 * reference point is when the system was started, and would wrap-around
 * every 49.7 days from that point because it returns a 32-bit value. This
 * wrap-around needs to be handled if it is relied on for delta measurements.
 */
u_int32 GetTickCount
	()
{
	struct timespec tickTime;

	clock_gettime(CLOCK_MONOTONIC, &tickTime);

	return (tickTime.tv_sec * 1000) + (tickTime.tv_nsec / 1000000);
}

/**
 * This function allocates memory for a given string using DRVMemoryAllocate()
 * and then copies the given string to the newly allocated buffer. The caller
 * or other Calling Component must free this string by calling the
 * DRVMemoryFree() function. The pSourceString should point to a valid
 * null-terminated C string.
 */
u_char *CreateString
(
	const char *pSourceString
)
{
	char *buffer = NULL;
	int len;

	/* Check if the input source string is NULL. */
	if (pSourceString == NULL)
	{
		return NULL;
	}

	/* Find the length of the source string. */
	len = (int)strlen((char *)pSourceString);

	/* Check the length of the source string. */
	if (len == 0)
	{
		return NULL;
	}

	/* Allocate memory for the source string of size length and return the
	   pointer to memory in buffer */
	DRVMemoryAllocate(len + 1, (void **)&buffer);
	if (buffer == NULL)
	{
		return NULL;
	}

	/* Copy the contents of the source string to the newly allocated memory
	   pointed to by buffer. */
	strcpy(buffer, (char *)pSourceString);

	/* Return the pointer to the newly allocated memory, containing the copy
	   of source string. */
	return (unsigned char *)buffer;
}

#endif
/**
 * Initialize the parent message queue structure, mostly setting meta data to
 * NULL.
 */
void MsgQInit
(
	MsgQStruct *msgQStruct
)
{
	msgQStruct->msgQ = NULL;
	msgQStruct->msgQTail = NULL;
}

/**
 * Add a message to the message queue. A message queue is a FIFO queue,
 * therefore, we are simply adding to the end of the list.
 */
int MsgQAddMessage
(
	MsgQStruct *msgQStruct,
	int command,
	u_int32 uParam,
	void *pParam
)
{
	if (msgQStruct->msgQ != NULL)
	{
		/* The message queue is not empty. */

		/* @todo msgQTail must not be NULL and msgQTail->next must be. Do the
		   check and handle the error if they are, which would indicate that
		   the list is out of sync. For now we are assuming that it is
		   valid. */

		/* Allocate a new message item. */
		msgQStruct->msgQTail->next = (MsgQ *)malloc(sizeof(MsgQ));
		/* @todo Change this to use DRVMemoryAllocate(). Also, check the memory
		   allocation and handle the error. */

		/* Reset the tail. */
		msgQStruct->msgQTail = msgQStruct->msgQTail->next;

		/* Initialize the new node. */
		msgQStruct->msgQTail->command = command;
		msgQStruct->msgQTail->uParam = uParam;
		msgQStruct->msgQTail->pParam = pParam;
		msgQStruct->msgQTail->next = NULL;
	}
	else
	{
		/* Allocate a new message item. */
		msgQStruct->msgQ = (MsgQ *)malloc(sizeof(MsgQ));
		/* @todo Change this to use DRVMemoryAllocate(). Also, check the memory
		   allocation and handle the error. */

		/* Reset the tail. */
		msgQStruct->msgQTail = msgQStruct->msgQ;

		/* Initialize the new node. */
		msgQStruct->msgQ->command = command;
		msgQStruct->msgQ->uParam = uParam;
		msgQStruct->msgQ->pParam = pParam;
		msgQStruct->msgQ->next = NULL;
	}

	/* Return success! */
	return 0;
}

/**
 * Remove the first message from the message queue, essentially removing the
 * head of the list.
 */
int MsgQRemoveMessage
(
	MsgQStruct *msgQStruct
)
{
	MsgQ *msgQ = NULL;

	/* If there is only one item in the list, or the list is empty, we will
	   make sure that the tail gets udpated. */
	if (msgQStruct->msgQ == msgQStruct->msgQTail)
	{
		msgQStruct->msgQTail = NULL;
	}

	if (msgQStruct->msgQ != NULL)
	{
		/* Keep a reference to the second item in the list (or NULL). */
		msgQ = msgQStruct->msgQ->next;

		/* Free the message item. */
		free(msgQStruct->msgQ);
		/* @todo Change this to use DRVMemoryFree(). */

		/* Reset the head of the list. */
		msgQStruct->msgQ = msgQ;
	}

	/* Return success! */
	return 0;
}

/**
 * Remove all messages from the message queue.
 */
void MsgQClear
(
	MsgQStruct *msgQStruct
)
{
	/* Loop until the message queue is empty (head points to NULL). */
	while (msgQStruct->msgQ != NULL)
	{
		/* Pass the message queue parent structure, no need to handle the
		   return value because the parent enclosed it anyway. */
		if (MsgQRemoveMessage(msgQStruct) != 0)
		{
			/* Break because some error has occurred. */
			break;
		}
	}
}

/**
 *
 */
int GetMessage
(
	MsgQStruct *msgQStruct,
	MsgQMessage *msg
)
{
	if (msgQStruct->msgQ != NULL)
	{
		/* Set all the return values. */
		msg->command = msgQStruct->msgQ->command;
		msg->uParam = msgQStruct->msgQ->uParam;
		msg->pParam = msgQStruct->msgQ->pParam;

		/* Remove the message from the message queue */
		if (MsgQRemoveMessage(msgQStruct) != 0)
		{
			return -2;
		}
		else
		{
			/* Return success! A message is being returned. */
			return 1;
		}
	}

	/* There's no message to process! */
	return -1;
}

/**
 *
 */
void MsgQDeinit
(
	MsgQStruct *msgQStruct
)
{
	MsgQClear(msgQStruct);
}

#if 0
/**
*   Strip whitespace chars off end of given string and return s.
*/
char *StripRightWhiteSpace
(
	char *s
)
{
	char *p = s + strlen(s);

	while (p > s && isspace(*--p))
	{
		*p = '\0';
	}

	return s;
}

/**
* Return pointer to first non-whitespace char in the given string.
*/
char *StripLeftWhiteSpace
(
	const char *s
)
{
	while (*s && isspace(*s))
	{
		s++;
	}

	return (char *)s;
}

char *Trim
(
	char *strg
)
{
	char *string, *tstr;

	if (*strg == '\n')
	{
		return (strg);
	}

	// Left trim string
	tstr = strg;

	while (*tstr != '\n' && *tstr && *tstr <= ' ')
	{
		++tstr;	// skip leading spaces
	}

	if (*tstr == '\n' || *tstr == '\0')
	{
		memmove(strg, tstr, strlen(tstr) + 1);
		return (strg);
	}

	// Right trim string
	string = &strg[strlen(strg)];

	while (*string <= ' ' && string >= tstr)
	{
		--string;	// skip trailing spaces
	}

	*(++string) = '\0';
	memmove(strg, tstr, strlen(tstr) + 1);
	return (strg);
}	// Trim

/**
 * TrimCRLF()
 * Trim a given buffer to end at the first CR/LF
 */
u_int32 TrimCRLF
(
	char *pBuffer
)
{
	u_int32 n;
	u_int32 len;

	len = (u_int32)strlen(pBuffer);

	/* Remove CR and LF characters. */
	for (n = 0; n < len; n++)
	{
		if ((pBuffer[n] == 0x0D) || (pBuffer[n] == 0x0A))
		{
			pBuffer[n] = 0;
			break;
		}
	}

	return (u_int32)strlen(pBuffer);
}

/**
*   find the character in the given string.
*/
char *Find_Char
(
	const char *s,
	char c
)
{
	while (*s && *s != c)
	{
		s++;
	}

	return (char *)s;
}

/**
 *  IsWSpace()
 * Check if the given character is a whitespace
 */
s_int32 IsWSpace
(
	char c
)
{
	return ((' ' == c) || ('\t' == c));
}

/**
 * SkipLead: Skip leading whitespaces
 *
 * The buffer pointer will be updated to a non-white space
 * character and a new length will be returned.
 *
 * In case of an error the buffer will remain untouched
 * and the length will equal to input length.
 */
s_int32 SkipLead
(
	char **pBuffer,
	s_int32 len
)
{
	char *tmpBuffer = *pBuffer;

	if ((NULL == pBuffer) || (NULL == tmpBuffer))
	{
		return len;
	}

	while (tmpBuffer[0] != '\0')
	{
		/* Skip Space and Tab */
		if ((' ' == tmpBuffer[0]) || ('\t' == tmpBuffer[0]))
		{
			tmpBuffer++;

			/* We have crossed the buffer size so bail out */
			if ((tmpBuffer - *pBuffer) > len)
			{
				return len;
			}
		}
		else
		{
			/* We have found a non-whitespace character */
			break;
		}
	}

	/* Nothing was updated as there was no leading whitespace */
	if (tmpBuffer == *pBuffer)
	{
		return len;
	}

	*pBuffer = tmpBuffer;
	return (u_int32)strlen(tmpBuffer);
}

/**
 * StringCopyUsingDelimiter: Copy a string using delimiter
 *
 *  [IN] pBuffer     - Pointer to buffer which is terminated by a '\0'
 *  [IN] iBufLen     - Length of the buffer pointed to by the pointer
 *  [OUT] pValue     - Pointer to a buffer to store the copied value
 *  [IN_OUT] pValLen - Pointer to an integer
 *                     As input it represents the buffer size
 *                     As output it gives the length of the copied value
 *  [IN] delim       - The string copy will terminate when it finds this delimiter
 *  [IN] bIgnoreWS   - If set TRUE will ignore white spaces when copying the string
 */
s_int32 StringCopyUsingDelimiter
(
	char *pBuffer,
	s_int32 iBufLen,
	char *pValue,
	s_int32 *pValLen,
	char delim,
	s_int32 bIgnoreWS
)
{
	s_int32 i = 0;
	s_int32 iValLen = *pValLen;

	/*
	 * We have saved the buffer size for value now we can reset it
	 * so that an early exit will return '0' length
	 */
	*pValLen = 0;

	/* Copy the value into a buffer */
	while (1)
	{
		if (delim == pBuffer[i])
		{
			break;
		}

		if (!bIgnoreWS)
		{
			if (IsWSpace(pBuffer[i]))
			{
				break;
			}
		}

		/* Cases handled:
		 *   - 'Value' did not terminate causing input buffer underflow
		 *   - 'Value' did not terminate causing output buffer overflow
		 */
		if ((i >= iBufLen) || (i >= iValLen))
		{
			return 0;
		}

		pValue[i] = pBuffer[i];
		i++;
	}

	/* Terminate the value */
	pValue[i] = '\0';

	/* Update the value length (real size) */
	*pValLen = (s_int32)strlen(pValue);

	return 1;
}

unsigned int getInterfaceIPv4
(
	char *pIface,
	char *pIPv4,
	unsigned int addrSize
)
{
	unsigned int res = 0;
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;
	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, pIface, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	strncpy(pIPv4, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), addrSize);

	return res;
}

unsigned int getAllInterfaces
(
	InterfaceAddressStruct **ppIf,
	unsigned int *pCount
)
{
	unsigned int res = 0;
	unsigned int i;
	char line[ADDRESS_STRING_SIZE];
	InterfaceAddressStruct *pIf;
	unsigned int count = 0;

	FILE *fp;

	*pCount = 0;
	*ppIf = NULL;

	fp = fopen("/proc/net/if_inet6", "r");
	if (fp == NULL)
	{
		res = 1;
		return res;
	}

	/* First determine the number of interfaces */
	while (fgets(line, ADDRESS_STRING_SIZE, fp) != NULL)
	{
		count++;
	}

	rewind(fp);

	DRVMemoryAllocate(count * sizeof(InterfaceAddressStruct), (void **)&pIf);
	DRVMemorySet((void *)pIf, 0, count * sizeof(InterfaceAddressStruct));

	/* The /proc/net/if_inet6 file contains IPv6 addresses and looks like this:
	    00000000000000000000000000000001 01 80 10 80       lo
	    fe80000000000000021fd0fffea5e83d 02 40 20 80     eth0
	 */
	i = 0;

	while ((fgets((char *)line, ADDRESS_STRING_SIZE, fp) != NULL) && (i < count))
	{
		char *src = &line[0];
		char *dst = &(pIf[i].pIPv6[0]);
		int j = 0;

		// Parse IPv6 address
		while ((*src) && (*src != ' ') && (dst - &(pIf[i].pIPv6[0]) < ADDRESS_STRING_SIZE))
		{
			*dst = *src;
			dst++;
			src++;
			if ((src - &line[0]) % 4 == 0)
			{
				// IPv6 address will end in ':' as well...
				*dst = ':';
				dst++;
			}
		}

		// ...so put NULL instead
		dst--;
		*dst = 0;

		dst = &(pIf[i].pIfName[0]);

		// Skip 4 options
		for (j = 0; j < 4; j++)
		{
			while ((*src) && (*src == ' '))
			{
				src++;
			}

			while ((*src) && (*src != ' '))
			{
				src++;
			}
		}

		// Skip all spaces
		while ((*src) && (*src == ' '))
		{
			src++;
		}

		// Save interface name
		while ((*src) && (*src != ' ') && (*src != '\n') && (dst - &(pIf[i].pIfName[0]) < ADDRESS_STRING_SIZE))
		{
			*dst = *src;
			dst++;
			src++;
		}

		*dst = 0;

		getInterfaceIPv4(pIf[i].pIfName, pIf[i].pIPv4, ADDRESS_STRING_SIZE);

		i++;
	}

	fclose(fp);

	*pCount = count;
	*ppIf = pIf;

	return res;
}

#endif
