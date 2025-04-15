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

#ifndef __ECRIOMEDIADATATYPES_H__
#define __ECRIOMEDIADATATYPES_H__

/** @enum EcrioAudioCodecEnum
 *
 */
typedef enum
{
	EcrioAudioCodec_None = 0,
	EcrioAudioCodec_PCMU,				/**< G.711 u-Law codec conforming to RFC 4856. The period contained in the frame data will be dictated by the Media Framework, typically 20ms worth. */
	EcrioAudioCodec_L16,				/**< Linear 16-bit PCM codec conforming to RFC 4856. emphasis and channel-order are not supported. */
	EcrioAudioCodec_AMR,				/**< AMR narrowband codec conforming to RFC 4867 and TS 26.101. The frame data is expected to hold the AMR Core Frame only. */
	EcrioAudioCodec_AMR_WB,				/**< AMR-WB wideband codec conforming to RFC 4867 and TS 26.201. The frame data is expected to hold the AMR Core Frame only. */
	EcrioAudioCodec_DTMF,				/**< DTMF codec using telephone-event payload conforming to RFC 4733. */
	EcrioAudioCodec_Last
} EcrioAudioCodecEnum;

/** @enum EcrioAudioCodecAMRParamStruct
 * This structure is used to specify capability of supporting AMR-NB and AMR-WB
 * specified parameters.
 */
typedef struct
{
	u_int32 uInitialBitrate;			/**< The initial bitrate to use for the codec (or mode if applicable for the given codec). */
	BoolEnum bIsUseAnswerOnly;			/**< If TRUE, this parameter is used to compare in SDP answer only, when received SDP offer. */
	u_int32 uSettings;					/**< Bit fields of settings for the given codec to configure locally. For AMR codecs, made up of one or more values from EcrioAudioCodecAMRStruct. */
} EcrioAudioCodecAMRParamStruct;

/** @enum EcrioAudioCodecDTMFParamStruct
 * This structure is used to specify capability of supporting DTMF event
 * indicated by digits.
 * @note We currently support only a single range of digits such as '0-11' or '0-15'.
 */
typedef struct
{
	u_int8 uDTMFEventStart;				/**< Start digit for DTMF events, inclusive. */
	u_int8 uDTMFEventStop;				/**< Stop digit for DTMF events, inclusive. */
} EcrioAudioCodecDTMFParamStruct;

/** @struct EcrioAudioFormatStruct
 * A structure used to specify settings for the basic audio format properties
 * such as sampling rate, resolution, channels, and period that the audio
 * subsystem should be configured for.
 */
typedef struct
{
	u_int32 uSampleRate;					/**< The desired sample rate, in Hertz (cycles per second). */
	u_int32 uChannels;						/**< The desired number of channels. */
} EcrioAudioFormatStruct;

/** @enum EcrioAudioCodecAMRStruct
 * A structure used to specify settings for AMR/AMR-WB bit fields. This structure
 * is used for SDP negotiated parameters.
 */
typedef struct
{
	u_int32 uModeSet : 10;					/**< A bitmask holding all the modes that can be supported by the stream. For example, msb=>[00 0000 0100]<=lsb indicates that mode 2 should be used. The default would be where all bits are set to 1. */
	u_int32 uOctetAlign : 1;				/**< 0 to disable, 1 to enable. */
	u_int32 uModeChangePeriod : 1;			/**< 0 (1) means that the sender can change the codec mode at any time, 1 (2) means it can be changed every second frame block.  */
	u_int32 uModeChangeCapability : 1;		/**< 0 (1) means that the client is not capable of restricting, 1 (2) means that it can restrict. This is mostly a factor of the negotiation. This should always be set to 1 (2) for IMS clients. */
	u_int32 uModeChangeNeighbor : 1;		/**< 0 if the receiver should handle mode changes arbitrarily, 1 to change only to neighboring modes in the mode-set. */
	u_int32 uCRC : 1;						/**< 0 to disable, 1 to enable. */

	// @temp robust-sorting will not be supported in Light IMS, remove it.
	u_int32 uRobustSorting : 1;				/**< 0 to disable, 1 to enable. */

	u_int32 uMaxRed : 16;					/**< When sending, what the maximum redundancy can be. When receiving, what the maximum might be from the remote. Can be in the range 0 to 65535. */
} EcrioAudioCodecAMRStruct;

/** @struct EcrioAudioCodecStruct
 * This structure is used to group an assortment of data that is provided when
 * a codec is initialized and prepared for use.
 */
typedef struct
{
	EcrioAudioCodecEnum eCodec;				/**< The selected (negotiated) codec to use. */
	EcrioAudioFormatStruct format;			/**< The format to use when initializing audio. */
	u_int32 uInitialBitrate;				/**< The initial bitrate to use for the codec (or mode if applicable for the given codec). */
	u_int32 uSettings;						/**< Bitmask of settings for the given codec to configure locally. For AMR codecs, made up of one or more values from EcrioAudioCodecAMREnum. */
} EcrioAudioCodecStruct;

/** @struct EcrioAudioCodecListStruct
 * This structure defines a list that summarizes the capability of audio codecs.
 */
typedef struct
{
	EcrioAudioCodecEnum eCodec;							/**< encoding name. */
	u_int8 uPayloadType;								/**< payload type. */
	u_int32 uSamplingRate;								/**< sampling rate. */
	u_int32 uChannels;									/**< number of channels. */
	union
	{
		EcrioAudioCodecAMRParamStruct amr;				/**< AMR/AMR-WB codec parameters. */
		EcrioAudioCodecDTMFParamStruct dtmf;			/**< DTMF event parameters. */
	} u;
} EcrioAudioCodecListStruct;

/** @struct EcrioAudioConfigStruct
 * This structure is used to configure audio settings. When we support multiple
 * audio codecs, set number of codecs and array of audio codec list properly.
 */
typedef struct
{
	u_int16 uPtime;										/**< ptime. */
	u_int16 uMaxPtime;									/**< maxptime. */
	u_int32 uNoOfCodecs;								/**< number of audio codecs in the array. */
	EcrioAudioCodecListStruct *pCodecs;					/**< array of audio codec list. */
} EcrioAudioConfigStruct;

/** @struct EcrioMediaConfigStruct
 * This structure defines the configuration parameters about handling media
 * to operate successfully.
 */
typedef struct
{
	char *pUsername;									/**< User name used to the SDP description. */
	EcrioAudioConfigStruct audio;						/**< Audio configuration. */
} EcrioMediaConfigStruct;

#endif /* #ifndef __ECRIOMEDIADATATYPES_H__ */
