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
 * @file EcrioPAL_Log.cpp
 * @brief This is the implementation of the Ecrio PAL's Log Module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#if defined(__linux__) && !defined(ANDROID)
#include <bsd/string.h>
#endif

#include "EcrioPAL.h"

/** Logging defines and helper functions */
#define PFDLOGNAME    "ECRIO_LOG"

#define _ECRIO_LOG_BUFFER_LENGTH_256				256
#define _ECRIO_LOG_BUFFER_LENGTH_512				512
#define _ECRIO_LOG_BUFFER_LENGTH_1024			1024

#include <stdarg.h>
static void __ec_PFD_log
(
	char lvl,
	const char *m,
	...
)
{
	char tmp[_ECRIO_LOG_BUFFER_LENGTH_1024];
	va_list args;

	va_start(args, m);
	vsnprintf(tmp, _ECRIO_LOG_BUFFER_LENGTH_1024, m, args);
	va_end(args);
	printf("%s - %c - %s\n", PFDLOGNAME, lvl, tmp);
}

#ifndef ANDROID
#define PFDLOGD(m, ...)		__ec_PFD_log('D', m,##__VA_ARGS__);
#define PFDLOGI(m, ...)		__ec_PFD_log('I', m,##__VA_ARGS__);
#define PFDLOGV(m, ...)		__ec_PFD_log('V', m,##__VA_ARGS__);
#define PFDLOGW(m, ...)		__ec_PFD_log('W', m,##__VA_ARGS__);
#define PFDLOGE(m, ...)		__ec_PFD_log('E', m,##__VA_ARGS__);
#else
#include <android/log.h>
#define LOG_TAG "IOTA/lims"
#define PFDLOGD(m)		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", m);
#define PFDLOGI(m)		__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", m);
#define PFDLOGV(m)		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "%s", m);
#define PFDLOGW(m)		__android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", m);
#define PFDLOGE(m)		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", m);
#endif

/* Definitions. */
#define BINARY_DUMP_LINE_CHARS		24	/**< The number of characters to print out for each line of a binary dump. */

/** @enum EcrioPFDModuleStateEnum
 * Enumeration holding the module level initialization state.
 */
typedef enum
{
	ECRIO_PFD_MODULE_STATE_UNINITIALIZED = 0,	/**< The state is uninitialized as the Init() function has not completed. */
	ECRIO_PFD_MODULE_STATE_INITIALIZED,			/**< The state is initialized, since the Init() function was successful. */
	ECRIO_PFD_MODULE_STATE_DEINITIALIZING		/**< The state is deinitializing, since the Deinit() function has been called and is either executing or completed. */
} EcrioPFDModuleStateEnum;

/** @struct LogStruct
 * This is the internal structure maintained by the Log Module and represents
 * a Log Module instance. Memory is allocated for it when the Init() function
 * is called and it is what the Log Module Handle points to. All subsequent
 * interface functions receive this pointer as parameter input.
 */
typedef struct
{
	EcrioLogOutput logOutput;			/**< A copy of the output value provided to the DRVLogInit() function. */
	u_int32 uLogFormat;					/**< A copy of the format bitmask provided to the DRVLogInit() function. */
	u_int32 uLogLevel;					/**< A copy of the level bitmask provided to the DRVLogInit() function. */
	u_int32 uLogComponent;				/**< A copy of the component bitmask provided to the DRVLogInit() function. */
	u_int32 uLogType;					/**< A copy of the type bitmask provided to the DRVLogInit() function. */

	FILE *file;							/**< For file output, the Win32 file handle. */

	char prefix[_ECRIO_LOG_BUFFER_LENGTH_256];					/**< The buffer used to form the log message prefix. */
	char buffer[_ECRIO_LOG_BUFFER_LENGTH_512];					/**< The buffer used for storing log messages to be recorded. */

	char word[8];						/**< The buffer used during binary dump operations for each word. */
	char line[_ECRIO_LOG_BUFFER_LENGTH_1024];						/**< The buffer used during binary dump operations for each line. */

	volatile EcrioPFDModuleStateEnum initState;	/**< The overall module state. */
	pthread_mutex_t initCS;				/**< The initialization critical section, used to synchronize the initialization state of the module. */
	pthread_mutex_t operationalCS;		/**< The operational critical section, used to synchronize overall public function execution. */
	unsigned long size;					/**< stores the size of the file. */
	char fileName[_ECRIO_LOG_BUFFER_LENGTH_256];					/**< storing filename used to create the log file. */
} LogStruct;

/**
 * This function will write information to the log using the specific recording
 * mechanism requested by the Calling Component. The function currently
 * supports file output and socket output.
 */
static void LogWrite
(
	LogStruct *h,
	char *buffer,
	unsigned int length,
	EcrioLogLevel level
)
{
	if ((h->logOutput & KLogOutputFile) || (h->logOutput & KLogOutputFileTimestamp))
	{
		/* Dump the output to the file. */
		fwrite(buffer, sizeof(char), length, h->file);
		/* Ignoring the return value. */
		fflush(h->file);
	}
	else if (h->logOutput & KLogOutputSystem)
	{
		/** For Android, this is logcat. */
		switch (level)
		{
			case KLogLevelDebug:
			{ PFDLOGD(buffer);
			}
			break;

			case KLogLevelInfo:
			{ PFDLOGI(buffer);
			}
			break;

			case KLogLevelVerbose:
			{ PFDLOGV(buffer);
			}
			break;

			case KLogLevelWarning:
			{ PFDLOGW(buffer);
			}
			break;

			case KLogLevelError:
			{ PFDLOGE(buffer);
			}
			break;

			default:
			break;
		}
	}
	else
	{
		/* No valid output type specified. */
	}
}

/**
 * This function is used to format binary log dumps. A buffer is provided and
 * each byte will be processed. Currently, 24 characters will be output on
 * each line including the starting address of the characters for the line,
 * the hex value of each character, and then an ASCII representation of the
 * binary characters, as long as they are printable.
 */
static int LogDump
(
	LogStruct *h,
	unsigned char *buffer,
	unsigned int length,
	EcrioLogLevel level
)
{
	unsigned int i = 0;
	unsigned int j = 0;

	if ((h->uLogFormat & KLogFormatShowBinary) == 0)
	{
		/* Loop for all characters in the buffer and print out the binary
		           and ascii dumps. */
		while (j < length)
		{
			/* Print out the line address. */
			sprintf(h->line, "\t%08X: ", j);

			/* Print out the hex dump. */
			for (i = 0; (i < BINARY_DUMP_LINE_CHARS) && ((i + j) < length); i++)
			{
				sprintf(h->word, "%02X ", buffer[i + j]);
				strlcat(h->line, h->word, _ECRIO_LOG_BUFFER_LENGTH_1024);
			}

			/* Fill in the rest of the hex line with spaces. */
			while (i < BINARY_DUMP_LINE_CHARS)
			{
				strlcat(h->line, "   ", _ECRIO_LOG_BUFFER_LENGTH_1024);
				i++;
			}

			/* Print the hex/character separator. */
			strlcat(h->line, ": ", _ECRIO_LOG_BUFFER_LENGTH_1024);

			/* Print out the character dump. */
			for (i = 0; (i < BINARY_DUMP_LINE_CHARS) && ((i + j) < length); i++)
			{
				/* Only show printable characters! */
				if (isprint(buffer[i + j]))
				{
					sprintf(h->word, "%c", buffer[i + j]);
				}
				else
				{
					strcpy(h->word, " ");
				}

				strlcat(h->line, h->word, _ECRIO_LOG_BUFFER_LENGTH_1024);
			}

			/* Fill in the rest of the character line with spaces. */
			while (i < BINARY_DUMP_LINE_CHARS)
			{
				strlcat(h->line, " ", _ECRIO_LOG_BUFFER_LENGTH_1024);
				i++;
			}

			/* Go to the next line. */
			strlcat(h->line, "\r\n", _ECRIO_LOG_BUFFER_LENGTH_1024);

			/* Dump the output. */
			LogWrite(h, h->line, (unsigned int)strlen(h->line), level);

			j += i;
		}
	}
	else
	{
		/* Loop for all characters in the buffer and print out the ascii dumps
		   only. */

		unsigned int uCurrentLine = 0;	// Keep a tally so we don't exceed buffer size.
		bool lastReturn = false;		// Remember when the last carriage return is (usually before line feed!).

		/* Tab in to the line. */
		sprintf(h->line, "\t");
		uCurrentLine = 2;	// Include terminating null char.

		/* Print out the character dump. */
		for (i = 0; i < length; ++i)
		{
			if (uCurrentLine + 1 >= 1024)
			{
				sprintf(h->line, "Line buffer size exceeded!\r\n");
				LogWrite(h, h->line, (unsigned int)strlen(h->line), level);
				break;
			}

			/* Only show printable characters! */
			if (isprint(buffer[i]))
			{
				sprintf(h->word, "%c", buffer[i]);
				strlcat(h->line, h->word, _ECRIO_LOG_BUFFER_LENGTH_1024);

			}
			else
			{
				if ((buffer[i] != 13) && (buffer[i] != 10))
				{
					sprintf(h->word, "?");
					strlcat(h->line, h->word, _ECRIO_LOG_BUFFER_LENGTH_1024);
				}
			}

			uCurrentLine++;

			if (buffer[i] == 13)
			{
				lastReturn = true;
			}

			if (buffer[i] == 10)
			{
				if (lastReturn)
				{
					/* Go to the next line. */
					strlcat(h->line, "\r\n", _ECRIO_LOG_BUFFER_LENGTH_1024);

					/* Dump the output. */
					LogWrite(h, h->line, (unsigned int)strlen(h->line), level);

					/* Tab in to the line. */
					sprintf(h->line, "\t");
					uCurrentLine = 2;
				}

				lastReturn = false;
			}
		}
	}

	return 0;
}

/**
 *
 */
const char *getLogLevelName
(
	EcrioLogLevel level
)
{
	switch (level)
	{
		case KLogLevelNone:
		{ return "NONE";
		}

		case KLogLevelDebug:
		{ return "DEBUG";
		}

		case KLogLevelInfo:
		{ return "INFO";
		}

		case KLogLevelVerbose:
		{ return "VERBOSE";
		}

		case KLogLevelWarning:
		{ return "WARN";
		}

		case KLogLevelError:
		{ return "ERR";
		}

		default: return "UNKNOWN";
	}
}

/**
 *
 */
const char *getLogComponentName
(
	unsigned int uLogComponent
)
{
	/** Remove the modifier part (highest 4 bits). */
	switch (uLogComponent & 0x0FFFFFFF)
	{
		case KLogComponentNone:
		{ return "NONE";
		}

		case KLogComponentUndefined:
		{ return "UNDEFINED";
		}

		case KLogComponentGeneral:
		{ return "GENERAL";
		}

		case KLogComponentClient:
		{ return "CLIENT";
		}

		case KLogComponentMoIP:
		{ return "MOIP";
		}

		case KLogComponentSMS:
		{ return "SMS";
		}

		case KLogComponentPresence:
		{ return "PRESENCE";
		}

		case KLogComponentXDM:
		{ return "XDM";
		}

		case KLogComponentRCS:
		{ return "RCS";
		}

		case KLogComponentMedia:
		{ return "MEDIA";
		}

		case KLogComponentSignaling:
		{ return "SIGNALING";
		}

		case KLogComponentSIPStack:
		{ return "SIP";
		}

		case KLogComponentSigComp:
		{ return "SIGCOMP";
		}

		case KLogComponentSUE:
		{ return "SUE";
		}

		case KLogComponentSAC:
		{ return "SAC";
		}

		case KLogComponentIMS:
		{ return "IMS";
		}

		case KLogComponentSSC:
		{ return "SSC";
		}

		case KLogComponentGBAME:
		{ return "GBAME";
		}

		case KLogComponentLCC:
		{ return "LCC";
		}

		case KLogComponentLDC:
		{ return "LDC";
		}

		case KLogComponentCPM:
		{ return "CPM";
		}

		default: return "UNKNOWN";
	}
}

/**
 *
 */
const char *getLogComponentModName
(
	unsigned int uLogComponent
)
{
	/** Focus on the modifier part (highest 4 bits). */
	switch (uLogComponent & 0xF0000000)
	{
		case KLogComponent_ALC:
		{ return "_ALC";
		}

		case KLogComponent_Engine:
		{ return "_ENGINE";
		}

		case KLogComponent_Plugin:
		{ return "_PLUGIN";
		}

		default: return "";
	}
}

/**
 *
 */
const char *getLogTypeName
(
	unsigned int uLogType
)
{
	/** Remove the modifier part (highest 4 bits). */
	switch (uLogType & 0x0FFFFFFF)
	{
		case KLogTypeGeneral:
		{ return "GEN";
		}

		case KLogTypeFuncEntry:
		{ return "ENTRY";
		}

		case KLogTypeFuncExit:
		{ return "EXIT";
		}

		case KLogTypeFuncParams:
		{ return "PARAM";
		}

		case KLogTypeSIP:
		{ return "SIP";
		}

		case KLogTypeRTP:
		{ return "RTP";
		}

		case KLogTypeRTCP:
		{ return "RTCP";
		}

		case KLogTypeHTTP:
		{ return "HTTP";
		}

		case KLogTypeMetric:
		{ return "METRIC";
		}

		case KLogTypeMemory:
		{ return "MEM";
		}

		case KLogTypeMutex:
		{ return "MUTEX";
		}

		case KLogTypeState:
		{ return "STATE";
		}

		case KLogTypeCheckpoint:
		{ return "CHECK";
		}

		case KLogTypeIterative:
		{ return "ITER";
		}

		default: return "UNKNOWN";
	}
}

/**
 *
 */
const char *getLogTypeModName
(
	unsigned int uLogType
)
{
	/** Focus on the modifier part (highest 4 bits). */
	switch (uLogType & 0xF0000000)
	{
		case KLogType_Fine:
		{ return "_FINE";
		}

		case KLogType_Finer:
		{ return "_FINER";
		}

		case KLogType_Finest:
		{ return "_FINEST";
		}

		default: return "";
	}
}

/**
 * This function will build the prefix part of a log message. The prefix is
 * configured via the logOutput bitmask and can include a date stamp, time
 * stamp, tick stamp, thread ID, and the log level for the message.
 */
static void LogPrefix
(
	LogStruct *h,
	EcrioLogLevel level,
	unsigned int uLogComponent,
	unsigned int uLogType,
	char *prefix
)
{

	if ((h->uLogFormat & KLogFormatShowDate) ||
		(h->uLogFormat & KLogFormatShowTime))
	{
		/* If either the date or time stamps are desired, obtain the current
		local date and time. */
		struct tm *time = NULL;
		struct timeval tv;
		unsigned short msec = 0;

		gettimeofday(&tv, NULL);
		msec = (unsigned short)(tv.tv_usec / 1000);
		time = localtime(&tv.tv_sec);

		if (h->uLogFormat & KLogFormatShowDate)
		{
			if (h->uLogFormat & KLogFormatShowTime)
			{
				/* Add the date and time to the prefix (YYYY-MM-DDTHH:MM:SS.mmm format). */
				snprintf(prefix, _ECRIO_LOG_BUFFER_LENGTH_256, 
					"%.4d-%.2d-%.2d\t%.2d:%.2d:%.2d.%.3d\t",
					time->tm_year + 1900,
					time->tm_mon + 1,
					time->tm_mday,
					time->tm_hour,
					time->tm_min,
					time->tm_sec,
					msec);
			}
			else
			{
				/* Add the date to the prefix without the time (YYYY-MM-DD format). */
				snprintf(prefix, _ECRIO_LOG_BUFFER_LENGTH_256, 
					"%.4d-%.2d-%.2d\t",
					time->tm_year + 1900,
					time->tm_mon + 1,
					time->tm_mday);
			}
		}
		else
		{
			if (h->uLogFormat & KLogFormatShowTime)
			{
				/* Add the time to the prefix without the date (HH:MM:SS.mmm format). */
				snprintf(prefix, _ECRIO_LOG_BUFFER_LENGTH_256, 
					"%.2d:%.2d:%.2d.%.3d\t",
					time->tm_hour,
					time->tm_min,
					time->tm_sec,
					msec);
			}
		}
	}

	if (h->uLogFormat & KLogFormatShowMsTick)
	{
		struct timespec tickTime;
		char temp[_ECRIO_LOG_BUFFER_LENGTH_256];

		clock_gettime(CLOCK_MONOTONIC, &tickTime);

		/* Add the current millisecond tick count to the prefix. */
		snprintf(temp, _ECRIO_LOG_BUFFER_LENGTH_256, "%sms:%010u\t", prefix, (unsigned int)((tickTime.tv_sec * 1000) + (tickTime.tv_nsec / 1000000)));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowUsTick)
	{
		char temp[_ECRIO_LOG_BUFFER_LENGTH_256];
		struct timespec tickTime;

		clock_gettime(CLOCK_MONOTONIC, &tickTime);

		/* Add the current microsecond tick count to the prefix. */
		snprintf(temp, _ECRIO_LOG_BUFFER_LENGTH_256, "%sus:%010u\t", prefix, (unsigned int)((tickTime.tv_sec * 1000000) + (tickTime.tv_nsec / 1000)));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowThread)
	{
		char temp[_ECRIO_LOG_BUFFER_LENGTH_256];
		/* Add the thread id to the prefix. */
		snprintf(temp, _ECRIO_LOG_BUFFER_LENGTH_256, "%sth:%08x\t", prefix, (unsigned int)pthread_self());
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowLevel)
	{
		char temp[_ECRIO_LOG_BUFFER_LENGTH_256];
		/* Add the level description to the prefix. */
		snprintf(temp, _ECRIO_LOG_BUFFER_LENGTH_256, "%s%s\t", prefix, getLogLevelName(level));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowComponent)
	{
		char temp[_ECRIO_LOG_BUFFER_LENGTH_256];
		/* Add the component description to the prefix. */
		snprintf(temp, _ECRIO_LOG_BUFFER_LENGTH_256, "%s%s%s\t", prefix, getLogComponentName(uLogComponent), getLogComponentModName(uLogComponent));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowType)
	{
		char temp[_ECRIO_LOG_BUFFER_LENGTH_256];
		/* Add the type description to the prefix. */
		snprintf(temp, _ECRIO_LOG_BUFFER_LENGTH_256, "%s%s%s\t", prefix, getLogTypeName(uLogType), getLogTypeModName(uLogType));
		memcpy(&prefix[0], temp, 256);
	}

}

LOGHANDLE pal_LogInit
(
	void *pParameters,
	EcrioLogOutput logOutput,
	u_int32 uLogFormat,
	u_int32 uLogLevel,
	u_int32 uLogComponent,
	u_int32 uLogType
)
{
	LogStruct *h = NULL;

	/** This function checks all parameters passed to it. */
	/* Currently, only file, file with timestamp and limited file size and system output is supported. */
	if (((logOutput & KLogOutputFile) == 0) &&
		((logOutput & KLogOutputSystem) == 0) &&
		((logOutput & KLogOutputFileLimit) == 0) &&
		((logOutput & KLogOutputFileTimestamp) == 0))
	{
		/* logOutput does not specify file, file with timestamp, limited file size or system output, so return NULL. */
		return NULL;
	}

	if (logOutput & KLogOutputFile || logOutput & KLogOutputFileLimit) {
		if (strlen((char *) pParameters) > (_ECRIO_LOG_BUFFER_LENGTH_256 - 1)) {
			return NULL;
		}
	}

	/** Allocate memory for this instance of the Log Module. */
	h = (LogStruct *)malloc(sizeof(LogStruct));
	if (h == NULL)
	{
		/* Memory for the Log Module is not available. */
		return NULL;
	}
	memset(h, 0, sizeof(LogStruct));

	/* Initialize structure attributes. */
	h->logOutput = logOutput;
	h->uLogFormat = uLogFormat;
	h->uLogLevel = uLogLevel;
	h->uLogComponent = uLogComponent;
	h->uLogType = uLogType;

	h->file = NULL;

	h->initState = ECRIO_PFD_MODULE_STATE_UNINITIALIZED;

	if (h->logOutput & KLogOutputFile || h->logOutput & KLogOutputFileLimit)
	{
		/** We can output to a file, even if multiple output support is
		    specified since we only support one output at a time. For file
		    output, the pParameters pointer should point to a filename. */
		const char *pFilename = (const char *)pParameters;

		if (pFilename == NULL)
		{
			/* The pFilename is NULL, so return NULL. We need a valid pointer for
			   file output because that is the file name. */

			/* Free the memory allocated for the Log Module instance. */
			free(h);
			/* Ignoring the return value. */

			return NULL;
		}

		h->file = fopen((char *)pFilename, "w+");
		if (h->file == NULL)
		{
			/* Free the memory allocated for the Log Module instance. */
			free(h);
			/* Ignoring the return value. */

			return NULL;
		}

		strcpy(h->fileName, (char *)pFilename);
	}
	else if (h->logOutput & KLogOutputFileTimestamp)
	{
		/** We can output to a file, even if multiple output support is
		    specified since we only support one output at a time. For file
		    output, the pParameters pointer should point to a filename. */
		if (pParameters == NULL)
		{
			/* The pFilename is NULL, so return NULL. We need a valid pointer for
			   file output because that is the file name. */

			/* Free the memory allocated for the Log Module instance. */
			free(h);
			/* Ignoring the return value. */

			return NULL;
		}

		char *pTempFilename = NULL;
		char pFilename[_ECRIO_LOG_BUFFER_LENGTH_256];
		struct tm *sTime = NULL;
		struct timeval tv;
		int len = strlen((const char *)pParameters);
		pTempFilename = (char *)malloc((len + 1)*sizeof(char));
		if (pTempFilename == NULL)
		{
			/* The pFilename is NULL, so return NULL. We need a valid pointer for
			   file output because that is the file name. */

			/* Free the memory allocated for the Log Module instance. */
			free(h);
			/* Ignoring the return value. */

			return NULL;
		}

		strcpy(pTempFilename, (char *)pParameters);
		/* Get the current Local time. */
		pTempFilename[len - 4] = '\0';
		gettimeofday(&tv, NULL);
		sTime = localtime(&tv.tv_sec);
		snprintf(pFilename, _ECRIO_LOG_BUFFER_LENGTH_256, "%s_%.4d%.2d%.2d%.2d%.2d%.2d.txt", (const char *)pTempFilename, (sTime->tm_year + 1900), (sTime->tm_mon + 1), (sTime->tm_mday), (sTime->tm_hour), (sTime->tm_min), (sTime->tm_sec));
		h->file = fopen((char *)pFilename, "w+");
		if (h->file == NULL)
		{
			/* Free the memory allocated for the Log Module instance. */
			free(h);
			free(pTempFilename);
			/* Ignoring the return value. */
			return NULL;
		}

		strcpy(h->fileName, (char *)pTempFilename);
		// free the pTempfilename
		free(pTempFilename);
	}
	else if (h->logOutput & KLogOutputSystem)
	{
		/* This is logcat, nothing to do. */
	}
	else
	{
		/* No other output types are supported at this time. */

		/* Free the memory allocated for the Log Module instance. */
		free(h);
		/* Ignoring the return value. */

		return NULL;
	}

	/** Initialize the initialization critical section. */
	pthread_mutex_init(&h->initCS, NULL);
	/* Critical sections are guaranteed by the OS to be initialized. */

	/** Initialize the operational critical section. */
	pthread_mutex_init(&h->operationalCS, NULL);
	/* Critical sections are guaranteed by the OS to be initialized. */

	/** Set the state as initialized. */
	h->initState = ECRIO_PFD_MODULE_STATE_INITIALIZED;

	return (LOGHANDLE)h;
}

void pal_LogDeinit
(
	LOGHANDLE *handle
)
{
	LogStruct *h = NULL;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return;
	}

	/* For Deinit() functions, the Handle must NOT be NULL. */
	if (NULL == *handle)
	{
		return;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (LogStruct *)*handle;

	/* Enter the initialization critical section. */
	pthread_mutex_lock(&h->initCS);
	if (h->initState != ECRIO_PFD_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   Deinit() would/should be called. This is a corner case if
		   the Calling Component erroneously called Deinit() multiple
		   times. */
		pthread_mutex_unlock(&h->initCS);
		return;
	}

	/* Set the state to deinitializing to prevent any other interface
	   function from executing. */
	h->initState = ECRIO_PFD_MODULE_STATE_DEINITIALIZING;
	pthread_mutex_unlock(&h->initCS);

	/** Enter the operational critical section. */
	pthread_mutex_lock(&h->operationalCS);

	if (h->logOutput & KLogOutputFile)
	{
		if (h->file != NULL)
		{
			/** Close the file handle. */
			fclose(h->file);
			/* Ignoring the return value. */
		}
	}
	else if (h->logOutput & KLogOutputSystem)
	{
		/* This is logcat, nothing to do. */
	}

	/** Leave the operational critical section. */
	pthread_mutex_unlock(&h->operationalCS);

	/** Delete the critical sections. */
	pthread_mutex_destroy(&h->operationalCS);
	pthread_mutex_destroy(&h->initCS);

	/** Free the memory allocated for the Log Module instance. */
	free(h);
	/* Ignoring the return value. */

	/** Set the output parameter to NULL, to force NULL in the Calling
	    Component's context. */
	*handle = NULL;
}

void pal_LogMessage
(
	LOGHANDLE handle,
	EcrioLogLevel level,
	u_int32 uLogComponent,
	u_int32 uLogType,
	const char *format,
	...
)
{
	LogStruct *h = NULL;
	va_list args;
	int bufferUsed;
	int prefixUsed;
	int bufferAvailable;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (LogStruct *)handle;

	/** Only log the message if the level is supported. */
	if ((level & h->uLogLevel) == 0)
	{
		/** This message shouldn't be logged. */
		return;
	}

	/** Only log the message if the component is supported. */
	if (((uLogComponent & 0x0FFFFFFF) & (h->uLogComponent & 0x0FFFFFFF)) == 0)
	{
		/** This message shouldn't be logged. */
		return;
	}

	/** Only log the message if the component modifier is needed and supported. */
	if ((uLogComponent & 0xF0000000) > 0)
	{
		/** A component modifier is specified. */
		if (((uLogComponent & 0xF0000000) & (h->uLogComponent & 0xF0000000)) == 0)
		{
			/** This message shouldn't be logged. */
			return;
		}
	}

	/** Only log the message if the type is supported. */
	if (((uLogType & 0x0FFFFFFF) & (h->uLogType & 0x0FFFFFFF)) == 0)
	{
		/** This message shouldn't be logged. */
		return;
	}

	/** Only log the message if the type modifier is needed and supported. */
	if ((uLogType & 0xF0000000) > 0)
	{
		/** A type modifier is specified. */
		if (((uLogType & 0xF0000000) & (h->uLogType & 0xF0000000)) == 0)
		{
			/** This message shouldn't be logged. */
			return;
		}
	}

	/** The format string must not be NULL. */
	if (format == NULL)
	{
		return;
	}

	/* Enter the initialization critical section. */
	pthread_mutex_lock(&h->initCS);
	if (h->initState != ECRIO_PFD_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		pthread_mutex_unlock(&h->initCS);
		return;
	}

	pthread_mutex_unlock(&h->initCS);

	/** Enter the operational critical section. */
	pthread_mutex_lock(&h->operationalCS);
	/** The critical section ensures that the message is placed in to the log
	    file in the proper order, mainly because once the prefix timestamp is
	    set, the processing should continue until the message is written. */

	/* Initialize the prefix string to be empty. */
	h->prefix[0] = '\0';

	/** Build the prefix of the line. */
	LogPrefix(h, level, uLogComponent, uLogType, &h->prefix[0]);

	/* Add the prefix. */
	prefixUsed = sprintf(h->buffer, "%s", h->prefix);
	if (prefixUsed == -1)
	{
		/* Some other error with sprintf(). */

		/* Leave the operational critical section. */
		pthread_mutex_unlock(&h->operationalCS);

		return;
	}

	/* Add the properly formatted message. Accomodate a NULL terminator (the
	   -1 in the size adjustment) and CRLF (the -2 in the size adjustment). */
	bufferAvailable = sizeof(h->buffer) - prefixUsed - 1 - 2;

	va_start(args, format);
	bufferUsed = vsnprintf(&h->buffer[prefixUsed], bufferAvailable, format, args);
	va_end(args);

	/* If bufferUsed == bufferAvailable '\0' is not added, so we end-up
	   truncating, therefore we must use >= */
	if (bufferUsed >= bufferAvailable)
	{
		h->buffer[sizeof(h->buffer) - 21 - 1] = '\0';
		bufferUsed = sprintf(h->buffer, "%s%s", h->buffer, "... [Log Truncated]\r\n");
		if (bufferUsed == -1)
		{
			/* Some other error with sprintf(). */

			/* Leave the operational critical section. */
			pthread_mutex_unlock(&h->operationalCS);

			return;
		}

		/* Ensure that the buffer is fully null terminated. */
		h->buffer[sizeof(h->buffer) - 1] = '\0';
	}
	else if (bufferUsed < -1)
	{
		/* Some other error with _vsnprintf(). */

		/* Leave the operational critical section. */
		pthread_mutex_unlock(&h->operationalCS);

		return;
	}
	else
	{
		bufferUsed += prefixUsed;

		// @todo Appending EOL here, but should make sure it fits...
#ifdef ANDROID
		if (h->logOutput != KLogOutputSystem)
		{
			strcat(h->buffer, "\r\n");
			bufferUsed += 2;
		}
#else
		strcat(h->buffer, "\r\n");
		bufferUsed += 2;
#endif
	}

	/** Once we have a valid buffer, record the log message. */
	LogWrite(h, h->buffer, (unsigned int)bufferUsed, level);

	/** Leave the operational critical section. */
	pthread_mutex_unlock(&h->operationalCS);
}

void pal_LogMessageDump
(
	LOGHANDLE handle,
	EcrioLogLevel level,
	u_int32 uLogComponent,
	u_int32 uLogType,
	u_char *pBuffer,
	u_int32 length
)
{
	LogStruct *h = NULL;
	int bufferUsed;

	/** This function checks all parameters passed to it. */
	/* The pointer to the Handle must not be NULL. */
	if (handle == NULL)
	{
		return;
	}

	/* Set the handle pointer to the proper internal pointer. */
	h = (LogStruct *)handle;

	/** Be sure message dumps are supported. */
	if ((h->uLogFormat & KLogFormatShowDumps) == 0)
	{
		return;
	}

	/** The buffer pointer must not be NULL. */
	if (pBuffer == NULL)
	{
		return;
	}

	/** The size must be less than 8KB for logging a binary buffer. */
	if ((length == 0) ||
		(length > 8192))
	{
		return;
	}

	/** Only log the message if the level is supported. */
	if ((level & h->uLogLevel) == 0)
	{
		/** This message shouldn't be logged. */
		return;
	}

	/** Only log the message if the component is supported. */
	if (((uLogComponent & 0x0FFFFFFF) & (h->uLogComponent & 0x0FFFFFFF)) == 0)
	{
		/** This message shouldn't be logged. */
		return;
	}

	/** Only log the message if the component modifier is needed and supported. */
	if ((uLogComponent & 0xF0000000) > 0)
	{
		/** A component modifier is specified. */
		if (((uLogComponent & 0xF0000000) & (h->uLogComponent & 0xF0000000)) == 0)
		{
			/** This message shouldn't be logged. */
			return;
		}
	}

	/** Only log the message if the type is supported. */
	if (((uLogType & 0x0FFFFFFF) & (h->uLogType & 0x0FFFFFFF)) == 0)
	{
		/** This message shouldn't be logged. */
		return;
	}

	/** Only log the message if the type modifier is needed and supported. */
	if ((uLogType & 0xF0000000) > 0)
	{
		/** A type modifier is specified. */
		if (((uLogType & 0xF0000000) & (h->uLogType & 0xF0000000)) == 0)
		{
			/** This message shouldn't be logged. */
			return;
		}
	}

	/** Enter the initialization critical section. */
	pthread_mutex_lock(&h->initCS);
	if (h->initState != ECRIO_PFD_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		pthread_mutex_unlock(&h->initCS);
		return;
	}

	pthread_mutex_unlock(&h->initCS);

	/** Enter the operational critical section. */
	pthread_mutex_lock(&h->operationalCS);
	/** The critical section ensures that the message is placed in to the log
	    file in the proper order, mainly because once the prefix timestamp is
	    set, the processing should continue until the message is written. */

	/* Initialize the prefix string to be empty. */
	h->prefix[0] = '\0';

	/** Build the prefix of the line. */
	LogPrefix(h, level, uLogComponent, uLogType, &h->prefix[0]);

	/** Print out the dump begin line. */
	bufferUsed = sprintf(h->buffer, "%sLog Binary Dump Begin (%u bytes)...\r\n", h->prefix, length);
	if (bufferUsed == -1)
	{
		/* Some other error with sprintf(). */

		/* Leave the operational critical section. */
		pthread_mutex_unlock(&h->operationalCS);

		return;
	}

	LogWrite(h, h->buffer, (unsigned int)bufferUsed, level);

	/** Dump the binary buffer. */
	if (LogDump(h, pBuffer, length, level) != 0)
	{
		/* Leave the operational critical section. */
		pthread_mutex_unlock(&h->operationalCS);

		return;
	}

	/** Print out the dump end line. */
	bufferUsed = sprintf(h->buffer, "%sLog Dump End.\r\n", h->prefix);
	if (bufferUsed == -1)
	{
		/* Some other error with sprintf(). */

		/* Leave the operational critical section. */
		pthread_mutex_unlock(&h->operationalCS);

		return;
	}

	LogWrite(h, h->buffer, (unsigned int)bufferUsed, level);

	/** Leave the operational critical section. */
	pthread_mutex_unlock(&h->operationalCS);
}
