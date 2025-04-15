# Copyright (c) 2017 Ecrio, Inc. All Rights Reserved.
# 
# Provided as supplementary materials for Licensed Software.
# 
# This file contains Confidential Information of Ecrio, Inc. and its suppliers.
# Certain inventions disclosed in this file may be claimed within patents owned
# or patent applications filed by Ecrio or third parties. No part of this
# software may be reproduced or transmitted in any form or by any means or used
# to make any derivative work (such as translation, transformation or
# adaptation) without express prior written consent from Ecrio. You may not mark
# or brand this file with any trade name, trademarks, service marks, or product
# names other than the original brand (if any) provided by Ecrio. Any use of
# Ecrio's or its suppliers work, confidential information, patented inventions,
# or patent-pending inventions is subject to the terms and conditions of your
# written license agreement with Ecrio. All other use and disclosure is strictly
# prohibited.
# 
# Ecrio reserves the right to revise this software and to make changes in
# content from time to time without obligation on the part of Ecrio to provide
# notification of such revision or changes.
# 
# ECRIO MAKES NO REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF
# ERRORS OR THAT THE SOFTWARE IS SUITABLE FOR YOUR USE. THE SOFTWARE IS PROVIDED
# ON AN "AS IS" BASIS FOR USE AT YOUR OWN RISK. ECRIO MAKES NO WARRANTIES,
# TERMS OR CONDITIONS, EXPRESS OR IMPLIED,EITHER IN FACT OR BY OPERATION OF LAW,
# STATUTORY OR OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.
# 
# TO THE FULL EXTENT ALLOWED BY LAW, ECRIO ALSO EXCLUDES FOR ITSELF AND ITS
# SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING
# NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
# PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
# BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING OUT
# OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF ECRIO HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGES. IF THIS SOFTWARE IS PROVIDED ON A COMPACT DISK,
# THE OTHER SOFTWARE AND DOCUMENTATION ON THE COMPACT DISK ARE SUBJECT TO THE
# LICENSE AGREEMENT ACCOMPANYING THE COMPACT DISK.

cmake_minimum_required(VERSION 3.5)

# IMS module files
set(SOURCES ${SOURCES}
	src/ims/EcrioSigMgrTimer.h
	src/ims/EcrioSigMgrTransactionHandler.c
	src/ims/EcrioSigMgrTransactionHandler.h
	src/ims/EcrioSigMgrUtilities.h
	src/ims/EcrioSigMgrUtilities.c
	src/ims/EcrioTXN.h
	src/ims/EcrioTXNCoreApi.c
	src/ims/EcrioTXNExit.c
	src/ims/EcrioTXNInit.c
	src/ims/EcrioTXNInternal.c
	src/ims/EcrioTXNInternal.h
	src/ims/EcrioTxnMgrSAC.h
	src/ims/EcrioTXNSACHandler.c
	src/ims/EcrioTXNStruct.c
	src/ims/EcrioTXNTimerCallback.c
	src/ims/EcrioTXNUtilityApi.c
	src/ims/EcrioBase64.c
	src/ims/EcrioBase64.h
	src/ims/EcrioCommonEngine.h
	src/ims/EcrioDSCommon.h
	src/ims/EcrioDSList.c
	src/ims/EcrioDSListInterface.h
	src/ims/EcrioDSListInternal.h
	src/ims/EcrioMD5.c
	src/ims/EcrioMD5.h
	src/ims/EcrioSigMgrAuthenticationRspHandler.c
	src/ims/EcrioSigMgrCallbacks.c
	src/ims/EcrioSigMgrCallbacks.h
	src/ims/EcrioSigMgrCallControl.c
	src/ims/EcrioSigMgrCallControl.h
	src/ims/EcrioSigMgrCallControlInternal.h
	src/ims/EcrioSigMgrCallControlUAC.c
	src/ims/EcrioSigMgrCallControlUAS.c
	src/ims/EcrioSigMgrCommon.h
	src/ims/EcrioSigMgrDialogHandler.c
	src/ims/EcrioSigMgrDialogHandler.h
	src/ims/EcrioSigMgrIMSLibHandler.c
	src/ims/EcrioSigMgrIMSLibHandler.h
	src/ims/EcrioSigMgrInit.c
	src/ims/EcrioSigMgrInit.h
	src/ims/EcrioSigMgrInitImpl.c
	src/ims/EcrioSigMgrInstantMessage.c
	src/ims/EcrioSigMgrInstantMessage.h
	src/ims/EcrioSigMgrInstantMessageInternal.h
	src/ims/EcrioSigMgrInstantMessageUAC.c
	src/ims/EcrioSigMgrInstantMessageUAS.c
	src/ims/EcrioSigMgrInternal.h
	src/ims/EcrioSigMgrMacroDefination.h
	src/ims/EcrioSigMgrMD5.h
	src/ims/EcrioSigMgrPublish.c
	src/ims/EcrioSigMgrPublish.h
	src/ims/EcrioSigMgrPublishImpl.c
	src/ims/EcrioSigMgrPublishInternal.h
	src/ims/EcrioSigMgrPublishUAC.c
	src/ims/EcrioSigMgrRegister.c
	src/ims/EcrioSigMgrRegister.h
	src/ims/EcrioSigMgrRegisterImpl.c
	src/ims/EcrioSigMgrRegisterInternal.h
	src/ims/EcrioSigMgrSessionHandler.c
	src/ims/EcrioSigMgrSipMessage.h
	src/ims/EcrioSigMgrSipMessageForm.c
	src/ims/EcrioSigMgrSipMessageParse.c
	src/ims/EcrioSigMgrSipMessageParseImpl.c
	src/ims/EcrioSigMgrSocketCallbacks.h
	src/ims/EcrioSigMgrSocketEventHandler.c
	src/ims/EcrioSigMgrStructRelease.c
	src/ims/EcrioSigMgrStructReleaseImpl.c
	src/ims/EcrioSigMgrSubscribe.c
	src/ims/EcrioSigMgrSubscribe.h
	src/ims/EcrioSigMgrSubscribeInternal.h
	src/ims/EcrioSigMgrRefer.c
	src/ims/EcrioSigMgrRefer.h
	src/ims/EcrioSigMgrReferImpl.c
	src/ims/EcrioSigMgrReferInternal.h
	src/ims/EcrioSigMgrOptionsUAC.c
	src/ims/EcrioSigMgrOptionsUAS.c
	src/ims/EcrioSigMgrOptionsInternal.h
	src/ims/EcrioSigMgrOptions.c
	src/ims/EcrioSigMgrOptions.h	
)

# SDP module files
set(SOURCES ${SOURCES}
	src/sdp/EcrioSDPParseHeaderA_file-range.c
	src/sdp/EcrioSDPParseHeaderA_file-selector.c
	src/sdp/EcrioSDPParseHeaderA_fmtp.c
	src/sdp/EcrioSDPParseHeaderA_rtpmap.c
	src/sdp/EcrioSDPParseHeaderB.c
	src/sdp/EcrioSDPParseHeaderC.c
	src/sdp/EcrioSDPParseHeaderM.c
	src/sdp/EcrioSDPParseHeaderO.c
	src/sdp/EcrioSDPSession.c
	src/sdp/EcrioSDPUtility.c
	src/sdp/EcrioSDP.c
	src/sdp/EcrioSDP.h
	src/sdp/EcrioSDPForm.c
	src/sdp/EcrioSDPInternal.h
	src/sdp/EcrioSDPParse.c
	src/sdp/EcrioSDPParseHeaderA.c
	src/sdp/EcrioSDPParseHeaderA_accept-types.c
	src/sdp/EcrioSDPParseHeaderA_AMR_parameters.c
	src/sdp/EcrioSDPParseHeaderA_DTMF_parameters.c
	src/sdp/EcrioSDPParseHeaderA_file-date.c
	src/sdp/EcrioSDPParseHeaderA_precond_current.c
	src/sdp/EcrioSDPParseHeaderA_precond_desired.c
)

# SUE module files
set(SOURCES ${SOURCES}
	src/sue/EcrioSUESigMgrCallBacks.c
	src/sue/EcrioSUESigMgrCallBacks.h
	src/sue/EcrioSUESigMgrSendRegister.c
	src/sue/EcrioSUESigMgrSetParam.c
	src/sue/EcrioSUEStartInternal.c
	src/sue/EcrioSUEStopInternal.c
	src/sue/EcrioSUEStructRelease.c
	src/sue/EcrioSUEStructReleaseInternal.c
	src/sue/EcrioSUESubscribe.c
	src/sue/EcrioSUEUtility.c
	src/sue/EcrioSUEEngine.h
	src/sue/EcrioSUEExit.c
	src/sue/EcrioSUEExitInternal.c
	src/sue/EcrioSUEHandleIncomingSIPMessage.c
	src/sue/EcrioSUEInit.c
	src/sue/EcrioSUEInterface.h
	src/sue/EcrioSUEInternalDataTypes.h
	src/sue/EcrioSUEInternalFunctions.h
	src/sue/EcrioSUERegister.c
	src/sue/EcrioSUESigMgr.h
)

# LIMS files
set(SOURCES ${SOURCES}
	src/lims_NetworkStateChange.c
	src/lims_Register.c
	src/lims_RespondSession.c
	src/lims_AbortSession.c
	src/lims_SendPagerMessage.c
	src/lims_Deinit.c
	src/lims_Deregister.c
	src/lims_Handler.c
	src/lims_Init.c
	src/lims_internal.c
	src/lims_internal.h
	src/lims_SetNetworkID.c
	src/lims_SetGroupChatHost.c
	src/lims_SetUserAgent.c	
)

# khash files
set(SOURCES ${SOURCES}
	src/external/khash/khash.h
)

# yxml files
set(SOURCES ${SOURCES}
	src/external/yxml/yxml.h
	src/external/yxml/yxml.c
)

# Include files
set(SOURCES ${SOURCES}
	src/external/yxml/yxml.h
	src/external/yxml/yxml.c
)

# gZip files
set(SOURCES ${SOURCES}
	src/external/gzip/adler32.c
	src/external/gzip/crc32.c
	src/external/gzip/deflate.c
	src/external/gzip/gzclose.c
	src/external/gzip/ecrio_gz.c
	src/external/gzip/gzlib.c
	src/external/gzip/gzread.c
	src/external/gzip/gzwrite.c
	src/external/gzip/inffast.c
	src/external/gzip/inflate.c
	src/external/gzip/inftrees.c
	src/external/gzip/trees.c
	src/external/gzip/zutil.c
	src/external/gzip/EcriogZipCompress.c
	src/external/gzip/EcriogZipDeCompress.c
	src/external/gzip/crc32.h
	src/external/gzip/deflate.h
	src/external/gzip/ecrio_gz.h
	src/external/gzip/EcriogZip.h
	src/external/gzip/gzguts.h
	src/external/gzip/inffast.h
	src/external/gzip/inffixed.h
	src/external/gzip/inflate.h
	src/external/gzip/inftrees.h
	src/external/gzip/trees.h
	src/external/gzip/zconf.h
	src/external/gzip/zlib.h
	src/external/gzip/zutil.h
)

# OOM files
set(SOURCES ${SOURCES}
	oom/src/default_oom.c
	oom/include/EcrioOOM.h
)

# Public headers for building lims
set(SOURCES ${SOURCES}
	include/EcrioDataTypes.h
	include/EcrioPAL.h
	include/EcrioPAL_Common.h
	include/EcrioPAL_Log.h
	include/EcrioPAL_Memory.h
	include/EcrioPAL_Mutex.h
	include/EcrioPAL_Socket.h
	include/EcrioPAL_String.h
	include/EcrioPAL_Timer.h
	include/EcrioPAL_Utility.h
	include/EcrioSIPDataTypes.h
	include/EcrioSUEDataTypes.h
	include/EcrioIOTA.h
	include/lims.h
)

# Private headers for building lims
set(PRIVATE_HEADERS ${PRIVATE_HEADERS}
	src
	src/ims
	src/sdp
	src/sue
	src/external/khash
	src/external/yxml
	src/external/gzip
)

# Public headers for building lims and dependent components
set(PUBLIC_HEADERS ${PUBLIC_HEADERS}
	include
	oom/include
)
