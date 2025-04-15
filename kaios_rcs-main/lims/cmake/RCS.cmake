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

# CPIM files
set(SOURCES ${SOURCES}
	src/cpim/EcrioCPIM.c
	src/cpim/EcrioCPIM.h
	src/cpim/EcrioCPIMCommon.h
	src/cpim/EcrioCPIMForm.c
	src/cpim/EcrioCPIMParse.c
	src/cpim/EcrioCPIMParseMessage.c
)

# CPM files
set(SOURCES ${SOURCES}
	src/cpm/EcrioCPMCallback.c
	src/cpm/EcrioCPMDeinit.c
	src/cpm/EcrioCPM.h
	src/cpm/EcrioCPMInit.c
	src/cpm/EcrioCPMInternal.c
	src/cpm/EcrioCPMInternal.h
	src/cpm/EcrioCPMSendPagerMessage.c
	src/cpm/EcrioCPMutilities.c
	src/cpm/EcrioCPMFormIMDN.c
	src/cpm/EcrioCPMParseXML.c
	src/cpm/EcrioCPMParseMultipartMessage.c
	src/cpm/EcrioCPMFormXML.c
	src/cpm/EcrioCPMSendRCSChatbotCommand.c
	src/cpm/EcrioCPMSetNetworkParam.c
)

# IMDN files
set(SOURCES ${SOURCES}
	src/imdn/EcrioIMDNForm.c
	src/imdn/EcrioIMDNParse.c
	src/imdn/EcrioIMDN.c
	src/imdn/EcrioIMDN.h
	src/imdn/EcrioIMDNCommon.h
)



# Public headers for building lims
set(SOURCES ${SOURCES}
	include/EcrioCPMDataTypes.h
	include/EcrioUCEDataTypes.h
)

# Private headers for building lims
set(PRIVATE_HEADERS ${PRIVATE_HEADERS}
	src/cpim
	src/cpm
	src/imdn
	src/msrp/library
	src/msrp
	src/uce
)

# Private definitions for building lims
set(PRIVATE_DEFINES ${PRIVATE_DEFINES}
	ANSI_PROTO
	MSRP_BY_REFERENCE
	MSRP_MIME_PARSING
	MSRP_AUTHENTICATE
	STK_RELEASE
	MSRP_UNUSED_FUNCTIONS
	MSRP_NO_CALLBACK
	MSRP_MSRPMSGBODY
	MSRP_CORE_STACK_COMPATIBLE
)

# Public definitions for building lims and dependents
set(PUBLIC_DEFINES ${PUBLIC_DEFINES}
	ENABLE_RCS
)
