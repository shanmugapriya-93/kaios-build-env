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
 * @file EcrioPAL_Log.c
 * @brief This is a test version of Ecrio PAL's Log Module.
 */

// @note This is currently the same as the reference implementation, no test hooks are needed yet.

#if defined(WIN32)
/* Ignore deprecated function warnings. */
#pragma warning(disable: 4996)

/* Indicate which libraries this implementation needs. */
#pragma comment(lib, "winmm.lib")

/* Standard includes. */
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable:4201)
#include <mmsystem.h>
#pragma warning(default:4201)
#endif

#else
#if defined(__linux__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#endif
#endif

/** Logging defines and helper functions */
#define PALLOGNAME    "ECRIO_LOG"

#include <stdarg.h>
static void __ec_PAL_log
(
	char lvl,
	const char *m,
	...
)
{
	char tmp[1024];
	va_list args;

	va_start(args, m);
	vsprintf(tmp, m, args);
	va_end(args);
	printf("%s - %c - %s\n", PALLOGNAME, lvl, tmp);
}

#ifndef ANDROID
#define PALLOGD(m, ...)		__ec_PAL_log('D', m,##__VA_ARGS__);
#define PALLOGI(m, ...)		__ec_PAL_log('I', m,##__VA_ARGS__);
#define PALLOGV(m, ...)		__ec_PAL_log('V', m,##__VA_ARGS__);
#define PALLOGW(m, ...)		__ec_PAL_log('W', m,##__VA_ARGS__);
#define PALLOGE(m, ...)		__ec_PAL_log('E', m,##__VA_ARGS__);
#else
#include <android/log.h>
#define LOG_TAG "IOTA/lims"
#define PALLOGD(m)		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", m);
#define PALLOGI(m)		__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", m);
#define PALLOGV(m)		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "%s", m);
#define PALLOGW(m)		__android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", m);
#define PALLOGE(m)		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", m);
#endif

#include "EcrioPAL.h"

#if defined(__linux__)


#endif

/* Definitions. */
#define BINARY_DUMP_LINE_CHARS    24/**< The number of characters to print out for each line of a binary dump. */

/** @enum EcrioPALModuleStateEnum
 * Enumeration holding the module level initialization state.
 */
typedef enum
{
	ECRIO_PAL_MODULE_STATE_UNINITIALIZED = 0,	/**< The state is uninitialized as the Init() function has not completed. */
	ECRIO_PAL_MODULE_STATE_INITIALIZED,			/**< The state is initialized, since the Init() function was successful. */
	ECRIO_PAL_MODULE_STATE_DEINITIALIZING		/**< The state is deinitializing, since the Deinit() function has been called and is either executing or completed. */
} EcrioPALModuleStateEnum;

/** @struct LogStruct
 * This is the internal structure maintained by the Log Module and represents
 * a Log Module instance. Memory is allocated for it when the Init() function
 * is called and it is what the Log Module Handle points to. All subsequent
 * interface functions receive this pointer as parameter input.
 */
typedef struct
{
	EcrioLogOutput logOutput;			/**< A copy of the output value provided to the pal_LogInit() function. */
	u_int32 uLogFormat;					/**< A copy of the format bitmask provided to the pal_LogInit() function. */
	u_int32 uLogLevel;					/**< A copy of the level bitmask provided to the pal_LogInit() function. */
	u_int32 uLogComponent;				/**< A copy of the component bitmask provided to the pal_LogInit() function. */
	u_int32 uLogType;					/**< A copy of the type bitmask provided to the pal_LogInit() function. */

#if defined(WIN32)
	LARGE_INTEGER frequency;

	HANDLE file;						/**< For file output, the Win32 file handle. */
#else
#if defined(__linux__)
	FILE *file;							/**< For file output, the Win32 file handle. */
#endif
#endif

	char prefix[256];					/**< The buffer used to form the log message prefix. */
	char buffer[512];					/**< The buffer used for storing log messages to be recorded. */

	char word[8];						/**< The buffer used during binary dump operations for each word. */
	char line[1024];						/**< The buffer used during binary dump operations for each line. */

	volatile EcrioPALModuleStateEnum initState;	/**< The overall module state. */
#if defined(WIN32)
	CRITICAL_SECTION initCS;			/**< The initialization critical section, used to synchronize the initialization state of the module. */
	CRITICAL_SECTION operationalCS;		/**< The operational critical section, used to synchronize overall public function execution. */
#else
#if defined(__linux__)
	pthread_mutex_t initCS;				/**< The initialization critical section, used to synchronize the initialization state of the module. */
	pthread_mutex_t operationalCS;		/**< The operational critical section, used to synchronize overall public function execution. */
	unsigned long size;					/**< stores the size of the file. */
	char fileName[256];					/**< storing filename used to create the log file. */
#endif
#endif
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
#if defined(WIN32)
	if (h->logOutput & KLogOutputFile)
	{
		BOOL bReturn;
		DWORD written;

		/* Dump the output to the file. */
		bReturn = WriteFile(h->file, buffer, length, &written, NULL);
		/* Ignoring the return value. */
	}
#else
#if defined(__linux__)
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
			{ PALLOGD(buffer);
			}
			break;

			case KLogLevelInfo:
			{ PALLOGI(buffer);
			}
			break;

			case KLogLevelVerbose:
			{ PALLOGV(buffer);
			}
			break;

			case KLogLevelWarning:
			{ PALLOGW(buffer);
			}
			break;

			case KLogLevelError:
			{ PALLOGE(buffer);
			}
			break;
		}
	}
#endif
#endif
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
				strcat(h->line, h->word);
			}

			/* Fill in the rest of the hex line with spaces. */
			while (i < BINARY_DUMP_LINE_CHARS)
			{
				strcat(h->line, "   ");
				i++;
			}

			/* Print the hex/character separator. */
			strcat(h->line, ": ");

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

				strcat(h->line, h->word);
			}

			/* Fill in the rest of the character line with spaces. */
			while (i < BINARY_DUMP_LINE_CHARS)
			{
				strcat(h->line, " ");
				i++;
			}

			/* Go to the next line. */
			strcat(h->line, "\r\n");

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
#if defined(WIN32)
				sprintf(h->line, "%s%c", h->line, buffer[i]);
#else
#if defined(__linux__)
				sprintf(h->word, "%c", buffer[i]);
				strcat(h->line, h->word);
#endif
#endif
			}
			else
			{
				if ((buffer[i] != 13) && (buffer[i] != 10))
				{
#if defined(WIN32)
					sprintf(h->line, "%s?", h->line);
#else
#if defined(__linux__)
					sprintf(h->word, "?", buffer[i]);
					strcat(h->line, h->word);
#endif
#endif
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
					strcat(h->line, "\r\n");

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

		case KLogComponentCPM:
		{ return "CPM";
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

		case KLogComponentPAL:
		{ return "PAL";
		}

		case KLogComponentAudioPAL:
		{ return "AUDIOPAL";
		}

		case KLogComponentVideoPAL:
		{ return "VIDEOPAL";
		}

		case KLogComponentNetworkPAL:
		{ return "NETWORKPAL";
		}

		case KLogComponentPhonePAL:
		{ return "PHONEPAL";
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
#if defined(WIN32)
	if ((h->uLogFormat & KLogFormatShowDate) ||
		(h->uLogFormat & KLogFormatShowTime))
	{
		/* If either the date or time stamps are desired, obtain the current
		    local date and time. */
		SYSTEMTIME time = {0};

		GetLocalTime(&time);

		if (h->uLogFormat & KLogFormatShowDate)
		{
			if (h->uLogFormat & KLogFormatShowTime)
			{
				/* Add the date and time to the prefix (YYYY-MM-DDTHH:MM:SS.mmm format). */
				sprintf(prefix, "%04d-%02d-%02d\t%02d:%02d:%02d.%03d\t",
					time.wYear,
					time.wMonth,
					time.wDay,
					time.wHour,
					time.wMinute,
					time.wSecond,
					time.wMilliseconds);
			}
			else
			{
				/* Add the date to the prefix without the time (YYYY-MM-DD format). */
				sprintf(prefix, "%04d-%02d-%02d\t",
					time.wYear,
					time.wMonth,
					time.wDay);
			}
		}
		else
		{
			if (h->uLogFormat & KLogFormatShowTime)
			{
				/* Add the time to the prefix without the date (HH:MM:SS.mmm format). */
				sprintf(prefix, "%02d:%02d:%02d.%03d\t",
					time.wHour,
					time.wMinute,
					time.wSecond,
					time.wMilliseconds);
			}
		}
	}

	if (h->uLogFormat & KLogFormatShowMsTick)
	{
		/* Add the current millisecond tick count to the prefix. */
		sprintf(prefix, "%sms:%010u\t", prefix, timeGetTime());
	}

	if (h->uLogFormat & KLogFormatShowUsTick)
	{
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);

		/* Add the current microsecond tick count to the prefix. */
		if (h->frequency.QuadPart != 0)
		{
			/* Split the constant so precision isn't lost either way. */
			sprintf(prefix, "%sus:%010u\t", prefix, (unsigned int)((currentTime.QuadPart * 1000) / (h->frequency.QuadPart / 1000)));
		}
	}

	if (h->uLogFormat & KLogFormatShowThread)
	{
		/* Add the thread id to the prefix. */
		sprintf(prefix, "%sth:%08x\t", prefix, GetCurrentThreadId());
	}

	if (h->uLogFormat & KLogFormatShowLevel)
	{
		/* Add the level description to the prefix. */
		sprintf(prefix, "%s%s\t", prefix, getLogLevelName(level));
	}

	if (h->uLogFormat & KLogFormatShowComponent)
	{
		/* Add the component description to the prefix. */
		sprintf(prefix, "%s%s%s\t", prefix, getLogComponentName(uLogComponent), getLogComponentModName(uLogComponent));
	}

	if (h->uLogFormat & KLogFormatShowType)
	{
		/* Add the type description to the prefix. */
		sprintf(prefix, "%s%s%s\t", prefix, getLogTypeName(uLogType), getLogTypeModName(uLogType));
	}
#else
#if defined(__linux__)
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
				sprintf(prefix, "%.4d-%.2d-%.2d\t%.2d:%.2d:%.2d.%.3d\t",
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
				sprintf(prefix, "%.4d-%.2d-%.2d\t",
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
				sprintf(prefix, "%.2d:%.2d:%.2d.%.3d\t",
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
		char temp[256];

		clock_gettime(CLOCK_MONOTONIC, &tickTime);

		/* Add the current millisecond tick count to the prefix. */
		sprintf(temp, "%sms:%010u\t", prefix, (unsigned int)((tickTime.tv_sec * 1000) + (tickTime.tv_nsec / 1000000)));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowUsTick)
	{
		char temp[256];
		struct timespec tickTime;

		clock_gettime(CLOCK_MONOTONIC, &tickTime);

		/* Add the current microsecond tick count to the prefix. */
		sprintf(temp, "%sus:%010u\t", prefix, (unsigned int)((tickTime.tv_sec * 1000000) + (tickTime.tv_nsec / 1000)));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowThread)
	{
		char temp[256];
		/* Add the thread id to the prefix. */
		sprintf(temp, "%sth:%08x\t", prefix, (unsigned int)pthread_self());
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowLevel)
	{
		char temp[256];
		/* Add the level description to the prefix. */
		sprintf(temp, "%s%s\t", prefix, getLogLevelName(level));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowComponent)
	{
		char temp[256];
		/* Add the component description to the prefix. */
		sprintf(temp, "%s%s%s\t", prefix, getLogComponentName(uLogComponent), getLogComponentModName(uLogComponent));
		memcpy(&prefix[0], temp, 256);
	}

	if (h->uLogFormat & KLogFormatShowType)
	{
		char temp[256];
		/* Add the type description to the prefix. */
		sprintf(temp, "%s%s%s\t", prefix, getLogTypeName(uLogType), getLogTypeModName(uLogType));
		memcpy(&prefix[0], temp, 256);
	}
#endif
#endif
}

/**
 * Implementation of the pal_LogInit() function. See the EcrioPAL_Log.h
 * file for interface definitions.
 */
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
#if defined(WIN32)
	LogStruct *h = NULL;

	/** This function checks all parameters passed to it. */
	/* Currently, only file output is supported. */
	if ((logOutput & KLogOutputFile) == 0)
	{
		/* logOutput does not specify file output, so return NULL. */
		return NULL;
	}

	/** Allocate memory for this instance of the Log Module. */
	h = (LogStruct *)malloc(sizeof(LogStruct));
	if (h == NULL)
	{
		/* Memory for the Log Module is not available. */
		return NULL;
	}

	/* Initialize structure attributes. */
	h->logOutput = logOutput;
	h->uLogFormat = uLogFormat;
	h->uLogLevel = uLogLevel;
	h->uLogComponent = uLogComponent;
	h->uLogType = uLogType;

	h->file = NULL;

	h->initState = ECRIO_PAL_MODULE_STATE_UNINITIALIZED;

	if (h->uLogFormat & KLogFormatShowUsTick)
	{
		QueryPerformanceFrequency(&h->frequency);
	}

	if (h->logOutput & KLogOutputFile)
	{
		/** We can output to a file, even if multiple output support is
		    specified since we only support one output at a time. For file
		    output, the pParameters pointer should point to a filename. */
		const char *pFilename = (const char *)pParameters;

		if (pFilename == NULL)
		{
			/* The pFilename is NULL, so return NULL. We need a valid pointer for
			   file output because that is the file name. */

			free(h);

			return NULL;
		}

#ifdef UNICODE
		WCHAR filename[MAX_PATH] = {0};
		mbstowcs(filename, (char *)pFilename, __min(MAX_PATH - 1, strlen((char *)pFilename) + 1));
		h->file = CreateFile(&filename[0], GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
#else
		h->file = CreateFile((char *)pFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
#endif
		if (h->file == INVALID_HANDLE_VALUE)
		{
			free(h);
			return NULL;
		}
	}
	else
	{
		/* No other output types are supported at this time. */

		free(h);
		return NULL;
	}

	/** Initialize the initialization critical section. */
	InitializeCriticalSection(&h->initCS);
	/* Critical sections are guaranteed by the OS to be initialized. */

	/** Initialize the operational critical section. */
	InitializeCriticalSection(&h->operationalCS);
	/* Critical sections are guaranteed by the OS to be initialized. */

	/** Set the state as initialized. */
	h->initState = ECRIO_PAL_MODULE_STATE_INITIALIZED;

	return (LOGHANDLE)h;
#else
#if defined(__linux__)
	LogStruct *h = NULL;
	unsigned int returnValue = 0;

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

	/** Allocate memory for this instance of the Log Module. */
	h = (LogStruct *)malloc(sizeof(LogStruct));
	if ((h == NULL))
	{
		/* Memory for the Log Module is not available. */
		return NULL;
	}

	/* Initialize structure attributes. */
	h->logOutput = logOutput;
	h->uLogFormat = uLogFormat;
	h->uLogLevel = uLogLevel;
	h->uLogComponent = uLogComponent;
	h->uLogType = uLogType;

	h->file = NULL;

	h->initState = ECRIO_PAL_MODULE_STATE_UNINITIALIZED;

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
		char pFilename[256];
		struct tm *sTime = NULL;
		struct timeval tv;
		int len = strlen((const char *)pParameters);
		pTempFilename = (char *)malloc((len + 1)*sizeof(char));
		if ((pTempFilename == NULL))
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
		sprintf(pFilename, "%s_%.4d%.2d%.2d%.2d%.2d%.2d.txt", (const char *)pTempFilename, (sTime->tm_year + 1900), (sTime->tm_mon + 1), (sTime->tm_mday), (sTime->tm_hour), (sTime->tm_min), (sTime->tm_sec));
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
	h->initState = ECRIO_PAL_MODULE_STATE_INITIALIZED;

	return (LOGHANDLE)h;
#endif
#endif
}

/**
 * Implementation of the pal_LogDeinit() function. See the EcrioPAL_Log.h
 * file for interface definitions.
 */
void pal_LogDeinit
(
	LOGHANDLE *handle
)
{
#if defined(WIN32)
	BOOL bReturn;
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
	EnterCriticalSection(&h->initCS);
	if (h->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   Deinit() would/should be called. This is a corner case if
		   the Calling Component erroneously called Deinit() multiple
		   times. */
		LeaveCriticalSection(&h->initCS);
		return;
	}

	/* Set the state to deinitializing to prevent any other interface
	   function from executing. */
	h->initState = ECRIO_PAL_MODULE_STATE_DEINITIALIZING;
	LeaveCriticalSection(&h->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->operationalCS);

	if (h->logOutput & KLogOutputFile)
	{
		if (h->file != NULL)
		{
			/** Close the file handle. */
			bReturn = CloseHandle(h->file);
			/* Ignoring the return value. */
		}
	}

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->operationalCS);

	/** Delete the critical sections. */
	DeleteCriticalSection(&h->operationalCS);
	DeleteCriticalSection(&h->initCS);

	free(h);
	/** Set the output parameter to NULL, to force NULL in the Calling
	    Component's context. */
	*handle = NULL;
#else
#if defined(__linux__)
	int hReturn;
	LogStruct *h = NULL;
	unsigned int returnValue = 0;

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
	if (h->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
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
	h->initState = ECRIO_PAL_MODULE_STATE_DEINITIALIZING;
	pthread_mutex_unlock(&h->initCS);

	/** Enter the operational critical section. */
	pthread_mutex_lock(&h->operationalCS);

	if (h->logOutput & KLogOutputFile)
	{
		if (h->file != NULL)
		{
			/** Close the file handle. */
			hReturn = fclose(h->file);
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
#endif
#endif
}

/**
 * Implementation of the pal_LogMessage() function. See the EcrioPAL_Log.h
 * file for interface definitions.
 */
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

#if defined(WIN32)
	/* Enter the initialization critical section. */
	EnterCriticalSection(&h->initCS);
	if (h->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&h->initCS);
		return;
	}

	LeaveCriticalSection(&h->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->operationalCS);
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
		LeaveCriticalSection(&h->operationalCS);

		return;
	}

	/* Add the properly formatted message. Accomodate a NULL terminator (the
	   -1 in the size adjustment) and CRLF (the -2 in the size adjustment). */
	bufferAvailable = sizeof(h->buffer) - prefixUsed - 1 - 2;

	va_start(args, format);
	bufferUsed = _vsnprintf(&h->buffer[prefixUsed], bufferAvailable, format, args);
	va_end(args);

	if (bufferUsed == -1)
	{
		h->buffer[sizeof(h->buffer) - 21 - 1] = '\0';
		bufferUsed = sprintf(h->buffer, "%s%s", h->buffer, "... [Log Truncated]\r\n");
		if (bufferUsed == -1)
		{
			/* Some other error with sprintf(). */

			/* Leave the operational critical section. */
			LeaveCriticalSection(&h->operationalCS);

			return;
		}

		/* Ensure that the buffer is fully null terminated. */
		h->buffer[sizeof(h->buffer) - 1] = '\0';
	}
	else if (bufferUsed < -1)
	{
		/* Some other error with _vsnprintf(). */

		/* Leave the operational critical section. */
		LeaveCriticalSection(&h->operationalCS);

		return;
	}
	else
	{
		bufferUsed += prefixUsed;

		// @todo Appending EOL here, but should make sure it fits...
		strcat(h->buffer, "\r\n");
		bufferUsed += 2;
	}

	/** Once we have a valid buffer, record the log message. */
	LogWrite(h, h->buffer, (unsigned int)bufferUsed, level);

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->operationalCS);
#else
#if defined(__linux__)
	/* Enter the initialization critical section. */
	pthread_mutex_lock(&h->initCS);
	if (h->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
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
#endif
#endif
}

/**
 * Implementation of the pal_LogMessageDump() function. See the EcrioPAL_Log.h
 * file for interface definitions.
 */
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

#if defined(WIN32)
	/** Enter the initialization critical section. */
	EnterCriticalSection(&h->initCS);
	if (h->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
	{
		/* Not in the initialized state, there is no valid case where
		   a function API should be allowed to run if not yet
		   initialized. */
		LeaveCriticalSection(&h->initCS);
		return;
	}

	LeaveCriticalSection(&h->initCS);

	/** Enter the operational critical section. */
	EnterCriticalSection(&h->operationalCS);
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
		LeaveCriticalSection(&h->operationalCS);

		return;
	}

	LogWrite(h, h->buffer, (unsigned int)bufferUsed, level);

	/** Dump the binary buffer. */
	if (LogDump(h, pBuffer, length, level) != 0)
	{
		/* Leave the operational critical section. */
		LeaveCriticalSection(&h->operationalCS);

		return;
	}

	/** Print out the dump end line. */
	bufferUsed = sprintf(h->buffer, "%sLog Dump End.\r\n", h->prefix);
	if (bufferUsed == -1)
	{
		/* Some other error with sprintf(). */

		/* Leave the operational critical section. */
		LeaveCriticalSection(&h->operationalCS);

		return;
	}

	LogWrite(h, h->buffer, (unsigned int)bufferUsed, level);

	/** Leave the operational critical section. */
	LeaveCriticalSection(&h->operationalCS);
#else
#if defined(__linux__)
	/** Enter the initialization critical section. */
	pthread_mutex_lock(&h->initCS);
	if (h->initState != ECRIO_PAL_MODULE_STATE_INITIALIZED)
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
#endif
#endif
}
