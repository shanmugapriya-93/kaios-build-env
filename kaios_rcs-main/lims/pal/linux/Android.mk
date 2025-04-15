# Copyright (c) 2019 Ecrio, Inc. All Rights Reserved.
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

ifndef MODULE_EcrioPAL
MODULE_EcrioPAL = 1

ifndef ENABLE_QCMAPI
ENABLE_QCMAPI = 1
endif

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := pal

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
ifeq ($(ENABLE_QCMAPI),1)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../system/libhidl/base/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../out/soong/.intermediates/vendor/qcom/proprietary/commonsys-intf/data/imscmservice/2.0/com.qualcomm.qti.imscmservice@2.0_genc++_headers/gen
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../out/soong/.intermediates/vendor/qcom/proprietary/commonsys-intf/data/imscmservice/2.1/com.qualcomm.qti.imscmservice@2.1_genc++_headers/gen
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../out/soong/.intermediates/vendor/qcom/proprietary/commonsys-intf/data/imscmservice/2.2/com.qualcomm.qti.imscmservice@2.2_genc++_headers/gen
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../out/soong/.intermediates/system/libhidl/transport/base/1.0/android.hidl.base@1.0_genc++_headers/gen
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../out/soong/.intermediates/system/libhidl/transport/manager/1.0/android.hidl.manager@1.0_genc++_headers/gen
endif

LOCAL_SRC_FILES  := EcrioPAL.cpp
#LOCAL_SRC_FILES  += EcrioPAL_Internal.h
LOCAL_SRC_FILES  += EcrioPAL_Log.cpp
LOCAL_SRC_FILES  += EcrioPAL_Memory.cpp
LOCAL_SRC_FILES  += EcrioPAL_Mutex.cpp
LOCAL_SRC_FILES  += EcrioPAL_Socket.cpp
LOCAL_SRC_FILES  += EcrioPAL_String.cpp
LOCAL_SRC_FILES  += EcrioPAL_Timer.cpp
LOCAL_SRC_FILES  += EcrioPAL_Utility.cpp
LOCAL_SRC_FILES  += EcrioPFD_Shared.cpp
#LOCAL_SRC_FILES  += EcrioPFD_Shared.h
LOCAL_SRC_FILES  += EcrioPFD_Signal.cpp
ifeq ($(ENABLE_QCMAPI),1)
LOCAL_SRC_FILES  += EcrioPAL_QCM.cpp
endif
#LOCAL_SRC_FILES  += EcrioPFD_Signal.h
LOCAL_SRC_FILES  += ifaddr_android.cpp

LOCAL_LDLIBS     := -ldl

#ifeq ($(NDK_DEBUG),1)
LOCAL_CPPFLAGS     += -DENABLE_LOG -DANDROID
#LOCAL_CPPFLAGS += -g
#LOCAL_SANITIZE:=address
#endif

ifeq ($(ENABLE_QCMAPI),1)
LOCAL_CPPFLAGS  += -DENABLE_QCMAPI -DENABLE_QCMAPI_SLEEP
endif

LOCAL_CPPFLAGS     += -ffunction-sections -fdata-sections -fvisibility=hidden

include $(BUILD_STATIC_LIBRARY)

endif
