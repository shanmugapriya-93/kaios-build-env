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

# This CMake file generates the Main Test App for iota including the lims
# library dependency.
project (iota_test)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(IOTA_SANITIZER_OPTION STREQUAL "Address")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize-recover=address")
	set (CMAKE_EXE_LINKER_FLAGS_INIT "-fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer -fsanitize=leak")
elseif(IOTA_SANITIZER_OPTION STREQUAL "Thread")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread -fsanitize-recover=thread")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread -fsanitize-recover=thread")
	set (CMAKE_EXE_LINKER_FLAGS_INIT "-fsanitize=thread -fsanitize-recover=thread -fno-omit-frame-pointer")
elseif(IOTA_SANITIZER_OPTION STREQUAL "Memory")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=memory -fsanitize-recover=memory -fsanitize-memory-track-origins=2 -stdlib=libc++ -lc++abi")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=memory -fsanitize-recover=memory -fsanitize-memory-track-origins=2 -stdlib=libc++ -lc++abi")
	set (CMAKE_EXE_LINKER_FLAGS_INIT "-fsanitize=memory -fsanitize-recover=memory -fno-omit-frame-pointer -fsanitize-memory-use-after-dtor")
elseif(IOTA_SANITIZER_OPTION STREQUAL "Undefined")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
	set (CMAKE_EXE_LINKER_FLAGS_INIT "-fsanitize=undefined")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DENABLE_LOG")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DENABLE_LOG")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	if(NOT CROSS_TARGET STREQUAL "arm-linux-gnueabi" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
	endif()

	# For all GNU builds, always output a map file.
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=iota_test.map")

	if(CROSS_TARGET STREQUAL "x86_64-alpine-linux-musl" AND IOTA_BUILD_COVERAGE)
		message("Disabling Code Coverage on Alpine!")
		set(IOTA_BUILD_COVERAGE OFF)
	endif()

	if(IOTA_CODE_COVERAGE)
		# Only to be used with GNU compiler for now.
		message("Adding linker support for Code Coverage for iota_test!")
		# We only want the executable to link with coverage, not generate it.
		# @todo How to get the OOM coverage then?
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
	endif()

	if(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
		# Optimize for size.
		set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -ffunction-sections -fdata-sections")
		set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -ffunction-sections -fdata-sections")

		# Optimize for size, strip out debug info and unused code.
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -ffunction-sections -fdata-sections -Wl,--gc-sections")
	endif()
else()
	message(${CMAKE_SYSTEM_NAME})
endif()

set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE HEADERS
	"../include/*.h"
	"src/*.h"
	"../oom/include/*.h"
)

# Explicitly reference all the required source files we need for the executable.
add_executable(
	iota_test
	"src/iota_test.cpp"
	"src/iota_test_config.cpp"
	"src/iota_test_menu.cpp"
	"src/iota_test_utility.cpp"
	"../oom/src/default_oom.c"
	"external/inih/dist/ini.c"
	${HEADERS})

target_include_directories(iota_test PRIVATE external/inih/dist)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# For Windows test tools, we don't care about security warnings.
	target_compile_definitions(iota_test PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

# Add the lims library dependency.
add_subdirectory(".." "${CMAKE_CURRENT_BINARY_DIR}/lims")

# Add the PAL dependency.
if(CROSS_TARGET STREQUAL "arm-linux-gnueabi")
	add_subdirectory("../pal/stub" "${CMAKE_CURRENT_BINARY_DIR}/pal") 
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_subdirectory("../pal/linux" "${CMAKE_CURRENT_BINARY_DIR}/pal")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_subdirectory("../pal/windows" "${CMAKE_CURRENT_BINARY_DIR}/pal")
else()
	message("There is no PAL implementation for this platform!")
	return()
endif()

if(CROSS_TARGET STREQUAL "arm-linux-gnueabi")
	target_link_libraries(iota_test lims pal)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_link_libraries(iota_test lims pal pthread rt bsd)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	# Dependencies seem to be taken in reverse order. In this case, the thread library is the last dependency (from the find_package syntax above).
	target_link_libraries(iota_test lims pal Threads::Threads rt bsd)
else()
	target_link_libraries(iota_test lims pal)
endif()
