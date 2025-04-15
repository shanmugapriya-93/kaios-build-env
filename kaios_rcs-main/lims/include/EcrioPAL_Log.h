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
 * @file EcrioPAL_Log.h
 * @brief This is the header file for the public interface of the Ecrio PAL's
 * Log Module.
 *
 * The purpose of the Log Module is to provide logging capabilities at run-
 * time to aid in debugging. Ecrio Components call the Log Module's functions
 * to write messages to a log file or other mechanism. Each message can be
 * constrained by various conditions so that the frequency of log output can
 * be controlled at runtime.
 *
 * <b>Log Output</b>
 * Log Output options are provided to the pal_LogInit() function only and only
 * one item can be specified at a time. For example, logs can be either saved
 * to a file or output using a system specific mechanism, but not both.
 *
 * <b>Log Format</b>
 * Log Format options are provided to the pal_LogInit() function only but it is
 * a bitmask, allowing any number of format options to be specified. Format
 * options control how each log line appears. For example, the data and time
 * can be enabled or disabled, based on the format.
 *
 * <b>Log Level</b>
 * Log Level options are provided to both the pal_LogInit() and pal_LogMessage()
 * functions. In each case, only one item can be specified at a time. The level
 * is a high level mapping for showing a specific class of logs. The levels
 * include:
 *
 * Information - Standard information, probably the most common, includes
 * function entry/exit, etc.
 * Debug - Used for debugging algorithms and sequences
 * Verbose - Extra information along the way, maybe none, depends on component
 * and needs.
 * Warning - An operation can not be completed, or that something is not
 * expected, but not fatal to the overall component (could be tried again)
 * Error - An operation can not be completed, or that some really bad problem
 * has been encountered, with no recovery without re-init component
 *
 * <b>Log Component</b>
 * When provided to the pal_LogInit() function, it is a bitmask and any number
 * of items can be specified. When provided to the pal_LogMessage() function,
 * only one item can be selected along with up to one modifier. The modifier
 * is in a special 4-bit section of the definition. For components, the
 * modifier can be nothing, ALC, or Engine. For example, Ecrio's MoIP ALC
 * component would be "KLogComponentMoIP | KLogComponent_ALC", the ALC part
 * being the modifier. Remember, only one component and up to one component
 * modifier can be used at a time.
 *
 * <b>Log Type</b>
 * When provided to the pal_LogInit() function, it is a bitmask and any number
 * of items can be specified. When provided to the pal_LogMessage() function,
 * only one item can be selected along with up to one modifier. The modifier
 * is in a special 4-bit section of the definition. For types, the modifier
 * can be nothing, fine, finer, or finest. Every log needs a type, and in
 * general, the general type KLogTypeGeneral can be used. But any type can be
 * modified with up to one modifier, such as
 * "KLogTypeIterative | KLogType_Finer" which could be from code buried in
 * a nested loop. Remember, only one component and up to one component
 * modifier can be used at a time.
 *
 * <b>Multithreading</b>
 * On platforms that support multithreading, all Log Module API functions
 * must be thread safe. In situations that require it, the Log Module APIs can
 * be invoked from different threads, accessing the same Log Module instance,
 * without fail.
 */

#ifndef __ECRIOPAL_LOG_H__
#define __ECRIOPAL_LOG_H__

/* Include common PAL definitions for building. */
#include "EcrioPAL_Common.h"

/* Allow this module to be called directly from C++ compiled code. */
#ifdef __cplusplus
extern "C" {
#endif

/** @name Module Handles
 * \n Handles are void pointers which will point to structures known
 * only internally to the Module. The Calling Component should only
 * reference the returned handles by these defined types. The handles
 * supported by this module include:
 */
/*@{*/

/**
 * The Log handle refers to the overall Log Module instance that is
 * created after calling pal_LogInit().
 */
typedef void *LOGHANDLE;

/*@}*/

/** @enum EcrioLogOutput
 * This enumeration defines the different destinations for log output. Only
 * one output destination can be used at a time. Not all of the methods need
 * to be supported by an implementation.
 */
typedef enum
{
	KLogOutputNone			= 0x00000000,	/**< No log output. */
	KLogOutputFile			= 0x00000001,	/**< Standard output to a file. */
	KLogOutputFileTimestamp	= 0x00000002,	/**< Standard output to a file, appending a timestamp to the file name. */
	KLogOutputFileLimit		= 0x00000004,	/**< Output to a file, but rolling over after some pre-set value specific to the implementation. */
	KLogOutputFileSpan		= 0x00000008,	/**< Output to a file, but span multiple files, after some pre-set size for each is reached. The file names should include a timestamp. */
	KLogOutputWindow		= 0x00000010,	/**< Output to a dbug window or other mechanism that might be application specific. */
	KLogOutputSocket		= 0x00000020,	/**< Output to a socket so that the log output is broadcast to a socket listener somewhere else, perhaps on a different machine. */
	KLogOutputSerial		= 0x00000040,	/**< Output to a serial port. */
	KLogOutputSystem		= 0x00000080,	/**< Output to the system specific logging mechanism (such as logcat on Android). */
	KLogOutputStdOut		= 0x00000100,	/**< Output to standard out (stdout). */
	KLogOutputStdErr		= 0x00000200	/**< Output to standard error (stderr). */
} EcrioLogOutput;

/** @enum EcrioLogFormat
 * The format enumeration defines what is placed on each log line. By default,
 * only the actual log message should be shown. But many other fields can be
 * enabled by these settings.
 */
typedef enum
{
	KLogFormatNone			= 0x00000000,	/**< No log format. */
	KLogFormatShowDate		= 0x00000001,	/**< Show the date (YYYY-MM-DD). */
	KLogFormatShowTime		= 0x00000002,	/**< Show the time (HH:MM:SS.mmm). */
	KLogFormatShowMsTick	= 0x00000004,	/**< Show a millisecond timestamp. */
	KLogFormatShowUsTick	= 0x00000008,	/**< Show a microsecond timestamp, using higher precision timer if available. */
	KLogFormatShowThread	= 0x00000010,	/**< Show the thread ID. */
	KLogFormatShowLevel		= 0x00000020,	/**< Show the log level. */
	KLogFormatShowComponent	= 0x00000040,	/**< Show the log component and modifier. */
	KLogFormatShowType		= 0x00000080,	/**< Show the log type and modifier. */
	KLogFormatShowDumps		= 0x00000100,	/**< Overall control to enable/disable binary dumps. */
	KLogFormatShowBinary	= 0x00000200	/**< If set, shows both binary and ascii in dumps, otherwise only ascii is shown. */
} EcrioLogFormat;

#define KLogFormat_Legacy		0x00000333	/**< Legacy bitmask similar to what has been done in the past for Ecrio logging. */
#define KLogFormat_Standard		0x000001F3	/**< Standard bitmask to show relevant information overall. */

/** @enum EcrioLogLevel
 *  The level enumeration defines the level each log message belongs to. The
 *  levels represent different classifications of logs and correspond to
 *  levels used by many popular logging systems. Please refer to the
 *  introduction for meaningful uses of each level. Each log message will
 *  belong to one level.
 */
typedef enum
{
	KLogLevelNone		= 0x00000000,		/**< No log level, not valid. */
	KLogLevelDebug		= 0x00000001,		/**< Used for algorithmic and systematic output throughout code. */
	KLogLevelInfo		= 0x00000002,		/**< Used for general information when going through code sequences. */
	KLogLevelVerbose	= 0x00000004,		/**< Used for extraneous information. */
	KLogLevelWarning	= 0x00000008,		/**< Used for non-fatal problem conditions. */
	KLogLevelError		= 0x00000010		/**< Used for fatal problem conditions. */
} EcrioLogLevel;

#define KLogLevel_All    0xFFFFFFFF			/**< All levels are allowed. */

/** @enum EcrioLogComponent
 * The component enumeration defines most of the components developed by Ecrio.
 * This information is used mostly for debugging, providing the ability to
 * concentrate on a specific component. Each log message can only belong to
 * one component, although it can optionally be further refined by using a
 * component modifier, an option from the high 4 bits. Only one modifier can
 * be specified at a time.
 */
typedef enum
{
	KLogComponentNone		= 0x00000000,	/**< No log component, not valid. */
	KLogComponentUndefined	= 0x00000001,	/**< Undefined, should not be seen in released commercial code. */
	KLogComponentGeneral	= 0x00000002,	/**< The general case, for when components are utility or minor and do not deserve their own assignment.*/
	KLogComponentClient		= 0x00000004,	/**< Reserved for general client level logging. */
	KLogComponentMoIP		= 0x00000010,	/**< Ecrio's MoIP framework. */
	KLogComponentSMS		= 0x00000020,	/**< Ecrio's SMS framework. */
	KLogComponentPresence	= 0x00000040,	/**< Ecrio's presence component(s). */
	KLogComponentXDM		= 0x00000080,	/**< Ecrio's XDM component(s). */
	KLogComponentCPM		= 0x00000100,	/**< Ecrio's CPM component(s). */
	KLogComponentRCS		= 0x00000200,	/**< Ecrio's RCS component(s). */
	KLogComponentMedia		= 0x00000400,	/**< Ecrio's Multimedia framework. */
	KLogComponentSignaling	= 0x00000800,	/**< Ecrio's signaling component (User Agent Engine). */
	KLogComponentSIPStack	= 0x00001000,	/**< Ecrio's SIP stack. */
	KLogComponentSigComp	= 0x00002000,	/**< Ecrio's Sigcomp stack. */
	KLogComponentSUE		= 0x00004000,	/**< Ecrio's Start-up Engine component. */
	KLogComponentSAC		= 0x00008000,	/**< Ecrio's Service and Application Controller component. */
	KLogComponentIMS		= 0x00010000,	/**< Ecrio's IMS Library component. */
	KLogComponentPAL		= 0x00020000,	/**< General PAL logging. */
	KLogComponentAudioPAL	= 0x00040000,	/**< Audio PAL specific logging. */
	KLogComponentVideoPAL	= 0x00080000,	/**< Video PAL specific logging. */
	KLogComponentNetworkPAL	= 0x00100000,	/**< Network PAL specific logging. */
	KLogComponentPhonePAL	= 0x00200000,	/**< Phone PAL specific logging. */
	KLogComponentSSC		= 0x00400000,	/**< SSC ALC specific logging. */
	KLogComponentGBAME		= 0x00800000,	/**< GBAME specific logging. */
	KLogComponentLCC		= 0x01000000,	/**< Ecrio's LCC components (client and server). */
	KLogComponentLDC		= 0x02000000,	/**< Ecrio's LDC components (client and server). */
	KLogComponentUCE        = 0x04000000,	/**< UCE specific logging. */
	KLogComponent_ALC		= 0x10000000,	/**< Modifier, component is an ALC. */
	KLogComponent_Engine	= 0x20000000,	/**< Modifier, component is an Engine. */
	KLogComponent_Plugin	= 0x40000000	/**< Modifier, component is a Plugin. */
} EcrioLogComponent;

#define KLogComponent_All    0xFFFFFFFF		/**< All components are allowed. */

/** @enum EcrioLogType
 * The type enumeration defines more ways to classify log messages. Each log
 * message can only belong to one component, although it can optionally be
 * further refined by using a type modifier, an option from the high 4 bits.
 * Only one modifier can be specified at a time.
 */
typedef enum
{
	KLogTypeNone		= 0x00000000,		/**< No log type, not valid. */
	KLogTypeGeneral		= 0x00000001,		/**< General classification, the general case and the most common. */
	KLogTypeFuncEntry	= 0x00000002,		/**< Function entry logs (usually just a simple log item). */
	KLogTypeFuncExit	= 0x00000004,		/**< Function exit logs (usually includes the status code/result of the function.) */
	KLogTypeFuncParams	= 0x00000008,		/**< Function parameters, to show upon entry or at other times. */
	KLogTypeSIP			= 0x00000010,		/**< To dump SIP messages. */
	KLogTypeRTP			= 0x00000020,		/**< To dump RTP messages. */
	KLogTypeRTCP		= 0x00000040,		/**< To dump RTCP messages. */
	KLogTypeHTTP		= 0x00000080,		/**< To dump HTTP messages. */
	KLogTypeMemory		= 0x00000100,		/**< To show memory operations. */
	KLogTypeMutex		= 0x00000200,		/**< To show mutex operations. */
	KLogTypeState		= 0x00000400,		/**< To show state changes. */
	KLogTypeMetric		= 0x00010000,		/**< To show metric logs, used for measurements. */
	KLogTypeCheckpoint	= 0x00020000,		/**< To show checkpoint logs, used for QA and debugging. */
	KLogTypeIterative	= 0x00040000,		/**< To show iterative logs, used for logs from loops. */
	KLogType_Fine		= 0x10000000,		/**< Modifier, type is at a fine granularity. */
	KLogType_Finer		= 0x20000000,		/**< Modifier, type is at a finer granularity. */
	KLogType_Finest		= 0x40000000		/**< Modifier, type is at the finest granularity. */
} EcrioLogType;

#define KLogType_All	0xFFFFFFFF			/**< All types are allowed. */

/** @struct LogHostStruct
 * If the Log Module instance supports logging via a socket, then this
 * structure would be passed in to the pal_LogInit() function by reference in
 * the pParameters parameter.
 */
typedef struct
{
	u_char *pHostIP;			/**< The IP address of the host to send log messages to. */
	u_int16 hostPort;			/**< The port of the host to send log messages to. */
} LogHostStruct;

/**
 * This function is used to create a new Log Module instance and initialize
 * it. This function must be called first before any other Log Module API
 * is called. This function returns a Log Module handle which points to the
 * complete internal state of the instance. The Calling Component should not
 * attempt to free the handle or modify it in any way. The pal_LogDeinit()
 * function should be called to properly clean up the Log Module instance.
 *
 * @param[in] pParameters		Void pointer to information relevant to the
 *								specific output control values specified by
 *								the logOutput parameter. See the description
 *								for KLogOutput for more information. If this
 *								parameter is NULL, the function will return
 *								NULL, since parameters are needed for all
 *								output methods. The contents of the pointer
 *								will be fully consumed but not freed when the
 *								function returns.
 * @param[in] uLogOutput		The desired output mechanism to use when
 *								logging messages, which is only one of the
 *								EcrioLogOutput values.
 * @param[in] uLogFormat		The desired format to use when logging which
 *								is a bitmask of one or more EcrioLogFormat
 *								values.
 * @param[in] uLogLevel			The desired log levels to show, which is a
 *								bitmask of one or more EcrioLogLevel
 *								enumerations.
 * @param[in] uLogComponent		The desired components to show, which is a
 *								bitmask of one or more EcrioLogComponent
 *								enumerations.
 * @param[in] uLogType			The desired messages to focus on, which is a
 *								bitmask of one or more EcrioLogType
 *								enumerations.
 * @return Pointer to the Log handle or NULL if there was any error.
 */
LOGHANDLE pal_LogInit
(
	void *pParameters,
	EcrioLogOutput uLogOutput,
	u_int32 uLogFormat,
	u_int32 uLogLevel,
	u_int32 uLogComponent,
	u_int32 uLogType
);

/**
 * This function is used to de-initialize a Log Module instance. This
 * function will clean up all resources and free all memory used by the Log.
 * This function can only operate successfully on an initialized handle, one
 * that was returned from a call to pal_LogInit().
 *
 * Although an implementation may offer some built-in protection, an
 * application should ensure that no other API is called for this module after
 * this function has been called.
 *
 * @param[in,out] handle		Pointer to a Log Module instance handle. If
 *								the pointer is NULL or if the instance handle
 *								is NULL, then the function will return an
 *								error. If it is not NULL, an attempt will be
 *								made to clean up resources and memory used by
 *								the instance. The instance handle will be set
 *								to NULL before returning.
 * @return None.
 */
void pal_LogDeinit
(
	LOGHANDLE *handle
);

/**
 * This function will write the desired message to the log instance specified
 * by the call to pal_LogInit(). This function acts in a similar manner to the
 * standard "printf" function, although the implementation will control where
 * and how the information is ultimately written.
 *
 * The limits of the function are left up to the implementation. Therefore,
 * this interface does not put a limit on the size of the resulting string
 * before or after the final formatted string is generated.
 *
 * @param[in] handle			A Log Module instance handle. If the pointer
 *								is NULL, then the function will return without
 *								doing anything.
 * @param[in] level				The specific log level of the message. If the
 *								level is supported by the level bitmask
 *								provided to the pal_LogInit() function, then it
 *								will be recorded.
 * @param[in] uLogComponent		The specific component to log for. If the
 *								component is supported by the component
 *								bitmask provided to the pal_LogInit() function,
 *								then it will be recorded. The component can be
 *								comprised of the specific component as well as
 *								a modifier such as of KLogComponentALC or
 *								KLogComponentEngine.
 * @param[in] uLogType			A particular type this message is for and an
 *								optional modifier. For most logs, the typical
 *								value should be 1 (KLogTypeGeneral). The
 *								specific messages to focus on are specified by
 *								the bitmask provided to the pal_LogInit()
 *								function.
 * @param[in] format			The message string to use, decorated with
 *								a variable list of arguments.
 * @return None.
 */
void pal_LogMessage
(
	LOGHANDLE handle,
	EcrioLogLevel level,
	u_int32 uLogComponent,
	u_int32 uLogType,
	const char *format,
	...
);

/**
 * This function will write the provided binary buffer to the log instance
 * specified by the call to pal_LogInit(). The actual method of recording
 * the binary data to the log is implementation specific, but most methods
 * will print out the binary hex values and corresponding printable ASCII
 * characters.
 *
 * The limits of the function are left up to the implementation. Therefore,
 * this interface does not put a limit on the size of the binary buffer, but
 * the implementation certainly can.
 *
 * @param[in] handle			A Log Module instance handle. If the pointer
 *								is NULL, then the function will return without
 *								doing anything.
 * @param[in] level				The specific log level of the message. If the
 *								level is supported by the level bitmask
 *								provided to the pal_LogInit() function, then it
 *								will be recorded.
 * @param[in] uLogComponent		The specific component to log for. If the
 *								component is supported by the component
 *								bitmask provided to the pal_LogInit() function,
 *								then it will be recorded. The component can be
 *								comprised of the specific component as well as
 *								a modifier such as of KLogComponentALC or
 *								KLogComponentEngine.
 * @param[in] uLogType			A particular type this message is for and an
 *								optional modifier. For most logs, the typical
 *								value should be 1 (KLogTypeGeneral). The
 *								specific messages to focus on are specified by
 *								the bitmask provided to the pal_LogInit()
 *								function.
 * @param[in] pBuffer			Pointer to the binary buffer to dump. If the
 *								pointer is NULL, then the function will return
 *								without doing anything.
 * @param[in] length			The length of the binary buffer to dump.
 * @return None.
 */
void pal_LogMessageDump
(
	LOGHANDLE handle,
	EcrioLogLevel level,
	u_int32 uLogComponent,
	u_int32 uLogType,
	u_char *pBuffer,
	u_int32 length
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ECRIOPAL_LOG_H__ */
