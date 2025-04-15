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

# This CMake file generates the iota "lims" library build files.
project (lims C)


##################################
# Profile configuration
set(IOTA_ENABLE_VOLTE OFF CACHE BOOL "Enable VoLTE profile")
set(IOTA_ENABLE_RCS ON CACHE BOOL "Enable RCS profile")
set(IOTA_ENABLE_ALL OFF CACHE BOOL "Enable all profiles")

# If all profiles are enabled, force VOLTE and RCS to ON
if (IOTA_ENABLE_ALL)
	set(IOTA_ENABLE_VOLTE ON)
	set(IOTA_ENABLE_RCS ON)
endif()

# At least IOTA_ENABLE_RCS or IOTA_ENABLE_VOLTE must be enabled. If both are
# disabled, error is raised and build fails
if (NOT IOTA_ENABLE_VOLTE AND NOT IOTA_ENABLE_RCS)
	message(FATAL_ERROR "Both IOTA_ENABLE_VOLTE and IOTA_ENABLE_RCS are disabled. Enable at least one profile to continue")
endif()

include(cmake/COMMON.cmake)

# Include VOLTE features, if enabled
if (IOTA_ENABLE_VOLTE)
	include(cmake/VOLTE.cmake)
endif()

# Include RCS features, if enabled
if (IOTA_ENABLE_RCS)
	include(cmake/RCS.cmake)
endif()


##################################
# Coverage configuration
set(IOTA_BUILD_COVERAGE OFF CACHE BOOL "Enable or disable code coverage")
if(IOTA_BUILD_COVERAGE)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		# Only to be used with GNU compiler for now.
		message(STATUS "Enabling Code Coverage for lims!")
		set(IOTA_PRIVATE_COMPILE_OPTIONS ${IOTA_PRIVATE_COMPILE_OPTIONS} --coverage)
		set(IOTA_LINK_LIBRARIES ${IOTA_LINK_LIBRARIES} --coverage)
	endif()
endif()

##################################
# ASAN configuration
set(IOTA_BUILD_ASAN OFF CACHE BOOL "Enable or disable Address Sanitizer")
if(IOTA_BUILD_ASAN)
	if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
		# Only to be used with GNU compiler for now. ASAN setting must be global
		message(STATUS "Enabling ASAN for lims!")
		set(IOTA_PUBLIC_COMPILE_OPTIONS ${IOTA_PUBLIC_COMPILE_OPTIONS} -fsanitize=address -fno-omit-frame-pointer)
		set(IOTA_LINK_LIBRARIES ${IOTA_LINK_LIBRARIES} -fsanitize=address -fno-omit-frame-pointer)
	endif()
endif()


##################################
# Build types
set(CMAKE_BUILD_TYPE_VALUES Debug Release RelWithDebInfo MinSizeRel)

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Setting build type to Debug as none was specified.")
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

if(NOT ${CMAKE_BUILD_TYPE} IN_LIST CMAKE_BUILD_TYPE_VALUES)
	message(FATAL_ERROR "Incorrect build type CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}. Allowed values: ${CMAKE_BUILD_TYPE_VALUES}")
endif()


##################################
# Logging configuration

set(IOTA_LOG_LEVEL_VALUES None Error Warning Info Verbose Debug Trace)
set(IOTA_LOG_LEVEL "None" CACHE STRING "Set log level: ${IOTA_LOG_LEVEL_VALUES}")
set_property(CACHE IOTA_LOG_LEVEL PROPERTY STRINGS ${IOTA_LOG_LEVEL_VALUES})

if(NOT ${IOTA_LOG_LEVEL} IN_LIST IOTA_LOG_LEVEL_VALUES)
	message(FATAL_ERROR "Incorrect log level IOTA_LOG_LEVEL=${IOTA_LOG_LEVEL}. Allowed values: ${IOTA_LOG_LEVEL_VALUES}")
endif()

if(IOTA_LOG_LEVEL STREQUAL "None")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} IOTA_LOG_LEVEL_NONE)
elseif(IOTA_LOG_LEVEL STREQUAL "Error")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} IOTA_LOG_LEVEL_ERROR)
elseif(IOTA_LOG_LEVEL STREQUAL "Warning")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} IOTA_LOG_LEVEL_WARNING)
elseif(IOTA_LOG_LEVEL STREQUAL "Info")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} IOTA_LOG_LEVEL_INFO)
elseif(IOTA_LOG_LEVEL STREQUAL "Verbose")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} IOTA_LOG_LEVEL_VERBOSE)
elseif(IOTA_LOG_LEVEL STREQUAL "Debug")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} IOTA_LOG_LEVEL_DEBUG)
elseif(IOTA_LOG_LEVEL STREQUAL "Trace")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} IOTA_LOG_LEVEL_TRACE)
endif()

# Defining legacy ENABLE_LOG until new logging scheme is implemented
# This define is applicable globally
if(NOT IOTA_LOG_LEVEL STREQUAL "None")
	set(PUBLIC_DEFINES ${PUBLIC_DEFINES} ENABLE_LOG)
endif()


##################################
# Configure library
add_library(lims ${SOURCES})

target_include_directories(lims PRIVATE ${PRIVATE_HEADERS})
target_include_directories(lims PUBLIC ${PUBLIC_HEADERS})

target_compile_definitions(lims PRIVATE ${PRIVATE_DEFINES})
target_compile_definitions(lims PUBLIC ${PUBLIC_DEFINES})

if (IOTA_PRIVATE_COMPILE_OPTIONS)
	target_compile_options(lims PRIVATE ${IOTA_PRIVATE_COMPILE_OPTIONS})
endif()

if (IOTA_PUBLIC_COMPILE_OPTIONS)
	target_compile_options(lims PUBLIC ${IOTA_PUBLIC_COMPILE_OPTIONS})
endif()

if(DEPENDENCIES)
	add_dependencies(lims ${DEPENDENCIES})
endif()

if(IOTA_LINK_LIBRARIES)
	target_link_libraries(lims INTERFACE ${IOTA_LINK_LIBRARIES})
endif()
