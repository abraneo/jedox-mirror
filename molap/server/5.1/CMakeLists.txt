################################################################################
###
### Copyright (C) 2006-2013 Jedox AG
###
### This program is free software; you can redistribute it and/or modify it
### under the terms of the GNU General Public License (Version 2) as published
### by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
###
### This program is distributed in the hope that it will be useful, but WITHOUT
### ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
### FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
### more details.
###
### You should have received a copy of the GNU General Public License along with
### this program; if not, write to the Free Software Foundation, Inc., 59 Temple
### Place, Suite 330, Boston, MA 02111-1307 USA
###
### If you are developing and distributing open source applications under the
### GPL License, then you are free to use Palo under the GPL License.  For OEMs,
### ISVs, and VARs who distribute Palo with their products, and do not license
### and distribute their source code under the GPL, Jedox provides a flexible
### OEM Commercial License.
###
### \author Zurab Khadikov, Jedox AG, Freiburg, Germany
###
###
################################################################################

# print config message
message(STATUS "================== Configuring Jedox OLAP Server ==================")

# minimum required cmake version
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# set project name
project("palo" CXX C)

message(STATUS "set olap major version number with: -DMAJOR_VERSION=[Number]")
message(STATUS "set olap minor version number with: -DMINOR_VERSION=[Number]")
if(MAJOR_VERSION MATCHES "[0-9]" AND MINOR_VERSION MATCHES "[0-9]")
    set(PALO_MAJOR ${MAJOR_VERSION})
    set(PALO_MINOR ${MINOR_VERSION})
else(MAJOR_VERSION MATCHES "[0-9]" AND MINOR_VERSION MATCHES "[0-9]")
    # change palo version here when need
    set(PALO_MAJOR 5)
    set(PALO_MINOR 1)
endif(MAJOR_VERSION MATCHES "[0-9]" AND MINOR_VERSION MATCHES "[0-9]")

# set path for extra cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules/CMake")

# include cmake modules
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckHeaderSTDC)
include(TestBigEndian)
include(CheckCXXSourceCompiles)
include(CMakeDependentOption)
include(CheckIncludeFile)
include(CheckTypeExists)
include(UserFunctions)

# Parsing errors more efficiently extra for eclipse
if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
endif(CMAKE_COMPILER_IS_GNUCC)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
endif(CMAKE_COMPILER_IS_GNUCXX)

################################################################################
### check build system
################################################################################

if(WIN32)
option(CMAKE_SUPPRESS_REGENERATION "Suppress regeneration of build scripts during build" ON)
    if("${CMAKE_GENERATOR}" MATCHES "Win64")
        set(ARCH_HINT "x86_64")
        message(STATUS "build system is: ${CMAKE_SYSTEM}-${ARCH_HINT}")
    else("${CMAKE_GENERATOR}" MATCHES "Win64")
        set(ARCH_HINT "i386")
        message(STATUS "build system is: ${CMAKE_SYSTEM}-${ARCH_HINT}")
    endif("${CMAKE_GENERATOR}" MATCHES "Win64")
else(WIN32)
    if("${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "x86_64")
        set(ARCH_HINT "x86_64")
        set(OUT_LIB_SUFFIX "64")
        set(CMAKE_LIBRARY_ARCHITECTURE "x86_64-linux-gnu") 
        message(STATUS "build system is: ${CMAKE_SYSTEM}-${ARCH_HINT}")
    else("${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "x86_64")
        set(ARCH_HINT "i386")
        set(OUT_LIB_SUFFIX "")
        set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
        message(STATUS "build system is: ${CMAKE_SYSTEM}-${ARCH_HINT}")
    endif("${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "x86_64")
endif(WIN32)

message(STATUS "c compiler is: ${CMAKE_C_COMPILER_ID}")
message(STATUS "c++ compiler is: ${CMAKE_CXX_COMPILER_ID}")

################################################################################
### define variables
################################################################################

# set include files variable 
set(INCLUDES "")

# libs variables
set(LIBS "")
set(OPENSSL_LIBS "")
set(LIC_LIBS "")

################################################################################
### checks for programs
################################################################################

if(NOT WIN32)
find_package(BISON)
find_package(FLEX)
find_program(GPERF_FOUND gperf PATHS /usr/bin /usr/local/bin)
if(GPERF_FOUND)
    set(ENABLE_GPERF yes)
    message(STATUS "gperf found")
endif(GPERF_FOUND)

# tells the compiler to warn about all the things it finds unusual
check_c_compiler_flag(-Wall WallC)
check_cxx_compiler_flag(-Wall WallCXX)
if(WallC AND WallCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# check if C compiler doesn't accept -c and -o together
set(CCO "-c -o")
check_c_compiler_flag(${CCO} MINUS_C_MINUS_O)
if(MINUS_C_MINUS_O)
     set(NO_MINUS_C_MINUS_O YES)
else(MINUS_C_MINUS_O)
     set(NO_MINUS_C_MINUS_O NO)
endif(MINUS_C_MINUS_O)
endif(NOT WIN32)

################################################################################
#########################  USER OPTIONS  #######################################
################################################################################

################################################################################
### set project build type only linux 
################################################################################

if(NOT WIN32)
set(PROJECT_BUILD_TYPE "Release" CACHE STRING "Select a build type")
set_property(CACHE PROJECT_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "RelWithDebInfo")
set(CMAKE_BUILD_TYPE ${PROJECT_BUILD_TYPE})

# user can change CMAKE_BUILD_TYPE from command-line
if(NOT (CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
    message(WARNING "only [Release],[Debug] or [RelWithDebInfo] build types allowed")
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "use default build type - ${CMAKE_BUILD_TYPE}")
else(NOT (CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
    message(STATUS "build type - ${CMAKE_BUILD_TYPE}")
endif(NOT (CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
message(STATUS "to change project build type use -DPROJECT_BUILD_TYPE={Release|Debug|RelWithDebInfo}") 
endif(NOT WIN32)

# set build type flags 
if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CONSOLE /D_USRDLL /MP /DSECURITY_WIN32 /EHsc /fp:fast /nologo")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CONSOLE /D_USRDLL /MP /DSECURITY_WIN32 /EHsc /fp:fast /nologo")
    # add addition standart lib
    set(LIBS ${LIBS} secur32.lib ws2_32.lib) 
    # add additional compile flags
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /bigobj /MDd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj /MDd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi /MD")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /MD")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} /MD")
else(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -std=c++0x")
    else(CMAKE_BUILD_TYPE STREQUAL Release)
        if(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
            set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g3 -ggdb -O2 -D_DEBUG")
            set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -ggdb -O2 -D_DEBUG -std=c++0x")
        else(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
            set(CMAKE_C_FLAGS_DEBUG "-gdwarf-4 -fvar-tracking-assignments -O0 -D_DEBUG")
            set(CMAKE_CXX_FLAGS_DEBUG "-gdwarf-4 -fvar-tracking-assignments -O0 -D_DEBUG -std=c++0x")
        endif(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    endif(CMAKE_BUILD_TYPE STREQUAL Release)
endif(WIN32)

################################################################################
### option for 32bit/64bit compile only for linux
################################################################################

if(NOT WIN32)
    option(ENABLE_32BIT "force 32bit compilation [default=OFF]" OFF)
    if(ENABLE_32BIT STREQUAL ON)
        set(CMAKE_REQUIRED_FLAGS "-m32")
        check_c_compiler_flag(-m32 m32C)
        check_cxx_compiler_flag(-m32 m32CXX)
        if(m32C AND m32CXX)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
            set(ARCH_HINT "i386")
            set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")
            set(OUT_LIB_SUFFIX "")
            #message(STATUS "librarys are: ${CMAKE_LIBRARY_ARCHITECTURE}")
        else(m32C AND m32CXX)
            set(CMAKE_REQUIRED_FLAGS "")
            set(ENABLE_32BIT OFF)
            message(STATUS "no support for 32bit compilation!")
        endif(m32C AND m32CXX)
    else(ENABLE_32BIT STREQUAL ON)
        set(ENABLE_32BIT OFF)   
    endif(ENABLE_32BIT STREQUAL ON)
    message(STATUS "32bit compilation is [${ENABLE_32BIT}]")
    message(STATUS "to enable/disable force 32bit compilation add -DENABLE_32BIT={ON|OFF}")
endif(NOT WIN32)

################################################################################
### define output build paths only linux 
################################################################################

if(NOT WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/usr/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/usr/lib${OUT_LIB_SUFFIX})
    message(STATUS "binary output directory is: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    message(STATUS "library output directory is: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endif(NOT WIN32)

################################################################################
### check for GpuEngine source code
################################################################################

unset(GPU_ENGINE CACHE)
find_file(GPU_ENGINE GpuEngine.h "${PROJECT_SOURCE_DIR}/Library/EngineGpu/")
if(GPU_ENGINE)
    message(STATUS "GPU Engine source code found")
    set(GPU_ENGINE ON)
else(GPU_ENGINE)
    message(STATUS "GPU Engine source code not found")
    set(GPU_ENGINE OFF)
endif(GPU_ENGINE)

################################################################################
### option for gpu server if gpu source code was founded
################################################################################

cmake_dependent_option(ENABLE_GPU_SERVER "enable building of gpu server" OFF "GPU_ENGINE" ON)

set(COMPUTE_CAPABILITY "" CACHE STRING "Set CUDA Compute Capability" )
if(COMPUTE_CAPABILITY STREQUAL "")
	set(COMPUTE_CAPABILITY_DEFAULT -gencode arch=compute_20,code=sm_20 -gencode arch=compute_35,code=sm_35)
else()
	set(COMPUTE_CAPABILITY_DEFAULT ${COMPUTE_CAPABILITY})
endif()

if(GPU_ENGINE STREQUAL ON)
if(ARCH_HINT STREQUAL x86_64 AND ENABLE_GPU_SERVER STREQUAL ON)
message(STATUS "CUDA Compute Capability: ${COMPUTE_CAPABILITY_DEFAULT}")
    set(ENABLE_GPU_SERVER ON)
    set(CUDA_64_BIT_DEVICE_CODE ON)
    set(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE ON)	
    find_package(CUDA 5.0 REQUIRED)      
    if(CUDA_FOUND)
        # setup cuda
        set(CUDA_PROPAGATE_HOST_FLAGS "OFF")
        if(WIN32)
            #set(CUDA_VERBOSE_BUILD ON)
            set(CL_OPTION_RELEASE "\" /EHsc /W3 /nologo /O2 /Zi /MD \"")
            set(CL_OPTION_DEBUG "\" /EHsc /W3 /nologo /Zi /RTC1 /MDd  \"")
            set(CUDA_NVCC_FLAGS_RELEASE ${COMPUTE_CAPABILITY_DEFAULT} -O2 -DNDEBUG -Xcompiler ${CL_OPTION_RELEASE})
            set(CUDA_NVCC_FLAGS_RELWITHDEBINFO ${COMPUTE_CAPABILITY_DEFAULT} -O2 -DNDEBUG -Xcompiler ${CL_OPTION_RELEASE})
            set(CUDA_NVCC_FLAGS_DEBUG ${COMPUTE_CAPABILITY_DEFAULT} -O0 -g -G -D_DEBUG -Xcompiler ${CL_OPTION_DEBUG})
            install(FILES ${CMAKE_SOURCE_DIR}/cudart64_50_35.dll DESTINATION ./)
            message(STATUS ${CUDA_TOOLKIT_ROOT_DIR})
            find_library(CUDA_RT_LIB NAMES cudart PATHS ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64)
            message(STATUS ${CUDA_RT_LIB})
            if(CUDA_RT_LIB)
                set(LIBS ${LIBS} cudart.lib)
                link_directories( ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64)
            endif(CUDA_RT_LIB)
        else(WIN32)
            set(CUDA_NVCC_FLAGS ${COMPUTE_CAPABILITY_DEFAULT} -O2 -DNDEBUG)
            if(PROJECT_BUILD_TYPE STREQUAL Debug)
                set(CUDA_NVCC_FLAGS ${COMPUTE_CAPABILITY_DEFAULT} -O0 -g -G -D_DEBUG)
            endif(PROJECT_BUILD_TYPE STREQUAL Debug)
            #message(STATUS ${CUDA_TOOLKIT_ROOT_DIR})
            find_library(CUDA_RT_LIB NAMES cudart PATHS ${CUDA_TOOLKIT_ROOT_DIR}/lib64)
            #message(STATUS ${CUDA_RT_LIB})
            if(CUDA_RT_LIB)
                add_library(cudart SHARED IMPORTED)
                set_property(TARGET cudart PROPERTY IMPORTED_LOCATION ${CUDA_TOOLKIT_ROOT_DIR}/lib64/libcudart.so)
                set(LIBS ${LIBS} cudart)
            endif(CUDA_RT_LIB)
        endif(WIN32)   
        set(INCLUDES ${INCLUDES} ${CUDA_INCLUDE_DIRS})
        # Add a special target to clean nvcc generated files.
        CUDA_BUILD_CLEAN_TARGET()
    else(CUDA_FOUND) 
        set(ENABLE_GPU_SERVER OFF)
        message(WARNING "cuda toolkit not found, you need cuda => 5.0")
    endif(CUDA_FOUND)   
else(ARCH_HINT STREQUAL x86_64 AND ENABLE_GPU_SERVER STREQUAL ON)
    set(ENABLE_GPU_SERVER OFF)   
endif(ARCH_HINT STREQUAL x86_64 AND ENABLE_GPU_SERVER STREQUAL ON) 

message(STATUS "enable building of gpu server is [${ENABLE_GPU_SERVER}]")
message(STATUS "to enable/disable building of gpu server add -DENABLE_GPU_SERVER={ON|OFF}")

else(GPU_ENGINE STREQUAL ON)
set(ENABLE_GPU_SERVER OFF)
endif(GPU_ENGINE STREQUAL ON)

################################################################################
### check for License extension
################################################################################

unset(PALO_LIC CACHE)
find_file(PALO_LIC License.h "${PROJECT_SOURCE_DIR}/Library/License/")
if(PALO_LIC)
    message(STATUS "License extension found")
    set(ENABLE_LICENSE_EXT ON)
else(PALO_LIC)
    message(STATUS "License extension not found")
    set(ENABLE_LICENSE_EXT OFF)
endif(PALO_LIC)

################################################################################
### check svn lib directory
################################################################################

# Manual add svn lib root directory
set(SVN_LIB_ROOT "" CACHE STRING "path to svn lib root directory" )
if(SVN_LIB_ROOT)
    set(SVN_LIB_ROOT_DIR ${SVN_LIB_ROOT})   
endif(SVN_LIB_ROOT)

if(SVN_LIB_ROOT_DIR)
    message(STATUS "path to svn lib root directory:  ${SVN_LIB_ROOT_DIR}")
    set(LIB_BOOST_ROOT ${SVN_LIB_ROOT_DIR}/boost)
    set(LIB_OPENSSL_ROOT ${SVN_LIB_ROOT_DIR}/openssl)
    set(LIB_PERFTOOLS_ROOT ${SVN_LIB_ROOT_DIR}/perftools)
    set(LIB_ICU_ROOT ${SVN_LIB_ROOT_DIR}/icu)
    set(LIB_ZLIB_ROOT ${SVN_LIB_ROOT_DIR}/zlib)
else(SVN_LIB_ROOT_DIR)
    message(STATUS "path to svn lib root directory is not defined!")
    message(STATUS "add it with -DSVN_LIB_ROOT_DIR=[path to svn lib root directory]")
endif(SVN_LIB_ROOT_DIR)

################################################################################
### check BOOST
################################################################################

if(LIB_BOOST_ROOT)
    set(LIB_BOOST_ROOT_DIR ${LIB_BOOST_ROOT})
endif(LIB_BOOST_ROOT)

if(LIB_BOOST_ROOT_DIR)
    if(WIN32)
        # set boost needed boost library parameters
        set(Boost_USE_MULTITHREADED ON) 
        set(Boost_USE_STATIC_LIBS   ON)		
        set(BOOST_ROOT ${LIB_BOOST_ROOT_DIR})
        set(BOOST_LIBRARYDIR ${LIB_BOOST_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
        find_package(Boost 1.51 REQUIRED COMPONENTS thread system chrono regex)
        if(Boost_FOUND)
            #message(STATUS ${Boost_INCLUDE_DIRS})
            #message(STATUS ${BOOST_LIBRARYDIR})
            set(INCLUDES ${INCLUDES} ${Boost_INCLUDE_DIRS})
            link_directories(${Boost_LIBRARY_DIRS})
        else(Boost_FOUND)
            message(FATAL_ERROR "Boost is not found, add boost with -DLIB_BOOST_ROOT_DIR=[path to BOOST]!")
        endif(Boost_FOUND)
    else(WIN32)
    find_library(BOOST_LIBS NAMES boost_thread-mt boost_system-mt boost_regex-mt PATHS ${LIB_BOOST_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
    if(BOOST_LIBS)
        add_library(libboost_thread-mt SHARED IMPORTED)
        add_library(libboost_system-mt SHARED IMPORTED)
        add_library(libboost_regex-mt SHARED IMPORTED)
        set_property(TARGET libboost_thread-mt PROPERTY IMPORTED_LOCATION ${LIB_BOOST_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libboost_thread-mt.so)
        set_property(TARGET libboost_system-mt PROPERTY IMPORTED_LOCATION ${LIB_BOOST_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libboost_system-mt.so)
        set_property(TARGET libboost_regex-mt PROPERTY IMPORTED_LOCATION ${LIB_BOOST_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libboost_regex-mt.so)
        set(LIBS ${LIBS} libboost_thread-mt libboost_system-mt libboost_regex-mt)
        set(INCLUDES ${INCLUDES} ${LIB_BOOST_ROOT_DIR}/include)
        # set some config variables
        set(HAVE_LIBBOOST_THREAD YES)
        set(HAVE_BOOST_THREAD_HPP YES)
    else(BOOST_LIBS)
        message(FATAL_ERROR "Boost is not found, add boost with -DLIB_BOOST_ROOT_DIR=[path to BOOST]!")
    endif(BOOST_LIBS)
    endif(WIN32)	
else(LIB_BOOST_ROOT_DIR)
    message(WARNING "Add boost with -DLIB_BOOST_ROOT_DIR=[path to BOOST]!")
    message(STATUS "Try to use boost installed on system")
    unset(Boost_FOUND CACHE)
    find_package( Boost 1.43 REQUIRED COMPONENTS thread system)
    if(Boost_FOUND)
        set(Boost_USE_MULTITHREADED ON)    
        set(INCLUDES ${INCLUDES} ${Boost_INCLUDE_DIRS})
        set(LIBS ${LIBS} boost_thread-mt boost_system-mt boost_regex-mt) 
        set(HAVE_LIBBOOST_THREAD YES)
        set(HAVE_BOOST_THREAD_HPP YES)
    else(Boost_FOUND)
        message(FATAL_ERROR "Boost is not found, install it!")
    endif(Boost_FOUND)
endif(LIB_BOOST_ROOT_DIR)

################################################################################
### check openSSL
################################################################################

if(LIB_OPENSSL_ROOT)
    set(LIB_OPENSSL_ROOT_DIR ${LIB_OPENSSL_ROOT})
endif(LIB_OPENSSL_ROOT)

if(LIB_OPENSSL_ROOT_DIR)
    if(WIN32)
        # Add includes and openssl libraries manualy 
        # TODO: make it generaic		
        find_library(LIB_EAY NAMES libeay32 PATHS ${LIB_OPENSSL_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/static)
        find_library(SSL_EAY NAMES ssleay32 PATHS ${LIB_OPENSSL_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/static)
        set(INCLUDES ${INCLUDES}  ${LIB_OPENSSL_ROOT_DIR}/include)
        set(OPENSSL_LIBS libeay32.lib ssleay32.lib)
        set(LIBS ${LIBS} libeay32.lib)
        link_directories(${LIB_OPENSSL_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/static)
    else(WIN32)
    find_library(SSL_LIBRARY NAMES ssl crypto PATHS ${LIB_OPENSSL_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
    if(SSL_LIBRARY)
        add_library(libssl SHARED IMPORTED)
        add_library(libcrypto SHARED IMPORTED)
        set_property(TARGET libssl PROPERTY IMPORTED_LOCATION ${LIB_OPENSSL_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libssl.so)
        set_property(TARGET libcrypto PROPERTY IMPORTED_LOCATION ${LIB_OPENSSL_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libcrypto.so)	
        # set some config variables
        set(HAVE_OPENSSL_SSL_H TRUE)
        set(HAVE_SSL_LIB TRUE)
        set(INCLUDES ${INCLUDES}  ${LIB_OPENSSL_ROOT_DIR}/include)
        set(OPENSSL_LIBS libssl libcrypto)
        set(LIBS ${LIBS} libcrypto)
    else(SSL_LIBRARY)
        message(FATAL_ERROR "OpenSSL not available, add it with -DLIB_OPENSSL_ROOT_DIR=[path to openssl]")        
    endif(SSL_LIBRARY)
    endif(WIN32)
else(LIB_OPENSSL_ROOT_DIR)
    message(WARNING "OpenSSL not available, add it with -DLIB_OPENSSL_ROOT_DIR=[path to openssl]") 
    message(STATUS "try to use openSSL installed on system")
    # try to find the OpenSSL encryption library
    # WARNING: not static linking!
    unset(OPENSSL_FOUND CACHE)
    find_package(OpenSSL) 
    if(OPENSSL_FOUND)
        # check if include file exists
        check_include_file(openssl/ssl.h HAVE_OPENSSL_SSL_H)
        # checking for SSL_get_error in ssl
        check_library_exists(ssl SSL_get_error "" HAVE_SSL_LIB)
        set(INCLUDES ${INCLUDES} ${OpenSSL_INCLUDE_DIRS})         
        set(OPENSSL_LIBS ssl crypto)
        set(LIBS ${LIBS} crypto)
    else(OPENSSL_FOUND)
        message(FATAL_ERROR "OpenSSL not available, install it!")
    endif(OPENSSL_FOUND)
endif(LIB_OPENSSL_ROOT_DIR)

################################################################################
### enable using of tcmalloc from google perf tools
################################################################################

if(NOT WIN32)
if(LIB_PERFTOOLS_ROOT)
    set(LIB_PERFTOOLS_ROOT_DIR ${LIB_PERFTOOLS_ROOT})
endif(LIB_PERFTOOLS_ROOT)

if(LIB_PERFTOOLS_ROOT_DIR)
    find_library(TCMALLOC NAMES tcmalloc_minimal PATHS ${LIB_PERFTOOLS_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
    if(TCMALLOC)
        # add tcmalloc_minimal
        add_library(libtcmalloc_minimal SHARED IMPORTED)
        set_property(TARGET libtcmalloc_minimal PROPERTY IMPORTED_LOCATION ${LIB_PERFTOOLS_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libtcmalloc_minimal.so)
        set(LIBS ${LIBS} libtcmalloc_minimal)
    else(TCMALLOC)
        message(FATAL_ERROR "tcmalloc_minimal not available, add it with -DLIB_PERFTOOLS_ROOT_DIR =[path to perftools]") 
    endif(TCMALLOC)
else(LIB_PERFTOOLS_ROOT_DIR)  
    message(WARNING "tcmalloc not available, add it with -DLIB_PERFTOOLS_ROOT_DIR=[path to perftools]") 
    message(STATUS "try to use tcmalloc_minimal installed on system")
    # try to find the tcmalloc_minimal library
    unset(TCMALLOC CACHE)
    if(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
        check_library_exists("tcmalloc_minimal -m32" malloc "/lib;/lib32;/usr/lib;/usr/lib32" TCMALLOC)
    else(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
        check_library_exists("tcmalloc_minimal" malloc "" TCMALLOC)
    endif(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
    #message(STATUS ${TCMALLOC})
    if (TCMALLOC)
        set(LIBS ${LIBS} tcmalloc_minimal)
    else (TCMALLOC)
        message(FATAL_ERROR "tcmalloc_minimal not available, install it or add it with -DLIB_PERFTOOLS_ROOT_DIR=[path to perftools]!")
    endif (TCMALLOC)
endif(LIB_PERFTOOLS_ROOT_DIR)
endif(NOT WIN32)

################################################################################
### Enables mode for testing, e.g. Timer
################################################################################
option(ENABLE_TEST_MODE "Enables mode for testing, e.g. Timer" OFF)
mark_as_advanced(ENABLE_TEST_MODE)

################################################################################
### enable using of profiler from google perf tools
################################################################################

if(NOT WIN32)
option(ENABLE_GOOGLE_CPU_PROFILER "add -lprofiler for google cpu profiling [default=OFF]" OFF)
mark_as_advanced(ENABLE_GOOGLE_CPU_PROFILER)

if(ENABLE_GOOGLE_CPU_PROFILER STREQUAL ON)
if(LIB_PERFTOOLS_ROOT)
    set(LIB_PERFTOOLS_ROOT_DIR ${LIB_PERFTOOLS_ROOT})
endif(LIB_PERFTOOLS_ROOT)

if(LIB_PERFTOOLS_ROOT_DIR)
    find_library(GPROFILER NAMES profiler ProfilerStart PATHS ${LIB_PERFTOOLS_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
    if(GPROFILER)
        # add tcmalloc_minimal
        add_library(libprofiler SHARED IMPORTED)
        set_property(TARGET libprofiler PROPERTY IMPORTED_LOCATION ${LIB_PERFTOOLS_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libprofiler.so)
        set(LIBS ${LIBS} libprofiler)
    else(GPROFILER)
        message(FATAL_ERROR "google profiler_minimal not available, add it with -DLIB_PERFTOOLS_ROOT_DIR =[path to perftools]") 
    endif(GPROFILER)
else(LIB_PERFTOOLS_ROOT_DIR)  
    message(WARNING "google profiler not available, add it with -DLIB_PERFTOOLS_ROOT_DIR=[path to perftools]") 
    message(STATUS "try to use tcmalloc_minimal installed on system")
    # try to find the tcmalloc_minimal library
    unset(GPROFILER CACHE)
    if(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
        check_library_exists("profiler -m32" ProfilerStart "/lib;/lib32;/usr/lib;/usr/lib32" GPROFILER)
    else(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
        check_library_exists("profiler" ProfilerStart "" GPROFILER)
    endif(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
    message(STATUS ${GPROFILER})
    if (GPROFILER)
        set(LIBS ${LIBS} profiler)
    else (GPROFILER)
        message(FATAL_ERROR "google profiler not available, install it or add it with -DLIB_PERFTOOLS_ROOT_DIR=[path to perftools]!")
    endif (GPROFILER)
endif(LIB_PERFTOOLS_ROOT_DIR)
endif(ENABLE_GOOGLE_CPU_PROFILER STREQUAL ON)
endif(NOT WIN32)

################################################################################
### check for udev only linux should be taken from system
################################################################################

if(NOT WIN32)
    if(ENABLE_LICENSE_EXT STREQUAL ON)
        message(STATUS "try to use udev installed on system")
        # try to find the udev library
        unset(UDEV CACHE)
        if(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
            check_library_exists("udev -m32" udev_device_get_sysname "/lib;/lib32;/usr/lib;/usr/lib32" UDEV)
        else(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
            check_library_exists("udev" udev_device_get_sysname "" UDEV) 
        endif(ENABLE_32BIT STREQUAL ON AND ARCH_HINT STREQUAL i386)
        # message(STATUS ${UDEV})
        if(UDEV)
            # libudev is needed only in liblic.palo.so extension
            #set(LIBS ${LIBS} udev) 
            set(LIC_LIBS ${LIC_LIBS} udev)
        else(UDEV)
            message(FATAL_ERROR "udev not available, install it!")
        endif(UDEV)
    endif(ENABLE_LICENSE_EXT STREQUAL ON)
endif(NOT WIN32)

################################################################################
### check for icu
################################################################################

if(LIB_ICU_ROOT)
    set(LIB_ICU_ROOT_DIR ${LIB_ICU_ROOT})
endif(LIB_ICU_ROOT)

if(LIB_ICU_ROOT_DIR)
    if(WIN32)
        # Add includes and libraries for icu manualy 
        # TODO: make it generaic		
        find_library(ICU_IN_LIB NAMES icuin PATHS ${LIB_ICU_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
        find_library(ICU_UC_LIB NAMES icuuc PATHS ${LIB_ICU_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
        set(INCLUDES ${INCLUDES} ${LIB_ICU_ROOT_DIR}/include)
        set(LIBS ${LIBS} icuin.lib icuuc.lib)
        link_directories(${LIB_ICU_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})	
        set(ICU_BIN_DIR ${LIB_ICU_ROOT_DIR}/bin/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
        install(FILES 
            ${ICU_BIN_DIR}/icudt49.dll
            ${ICU_BIN_DIR}/icuin49.dll
            ${ICU_BIN_DIR}/icuuc49.dll
            DESTINATION ./)
    else(WIN32)
	    find_library(ICU NAMES icuuc icui18n PATHS ${LIB_ICU_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
	    if(ICU)
	        add_library(libicuuc SHARED IMPORTED)
	        add_library(libicui18n SHARED IMPORTED)
	        set_property(TARGET libicuuc PROPERTY IMPORTED_LOCATION ${LIB_ICU_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libicuuc.so)
	        set_property(TARGET libicui18n PROPERTY IMPORTED_LOCATION ${LIB_ICU_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libicui18n.so)
	        set(INCLUDES ${INCLUDES} ${LIB_ICU_ROOT_DIR}/include)
	        set(LIBS ${LIBS} libicuuc libicui18n)
	    else(ICU)
	        message(FATAL_ERROR "icu not available, add it with -DLIB_ICU_ROOT_DIR =[path to icu]") 
	    endif(ICU)
    endif(WIN32)
else(LIB_ICU_ROOT_DIR)  
    message(WARNING "icu not available, add it with -DLIB_ICU_ROOT_DIR=[path to icu]") 
    message(STATUS "try to use icu installed on system")
    # try to find the icu library
    unset(ICU_FOUND CACHE)
    find_package (ICU REQUIRED COMPONENTS uc i18n)
    if(ICU_FOUND)
        set(INCLUDES ${INCLUDES} ${ICU_INCLUDE_DIRS})
        set(LIBS ${LIBS} icuuc icui18n)
    else(ICU_FOUND)
        message(FATAL_ERROR "icu not available, install it or add it with -DLIB_ICU_ROOT_DIR =[path to icu]!")
    endif(ICU_FOUND)
endif(LIB_ICU_ROOT_DIR)

################################################################################
### check for zlib
################################################################################

if(LIB_ZLIB_ROOT)
    set(LIB_ZLIB_ROOT_DIR ${LIB_ZLIB_ROOT})
endif(LIB_ZLIB_ROOT)

if(LIB_ZLIB_ROOT_DIR)
    if(WIN32)
#	    find_library(ZLIB_MINI_LIB NAMES libminizip PATHS ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
#	    if(ZLIB_MINI_LIB)
#	        add_library(minizip STATIC IMPORTED)
#	        set_property(TARGET minizip PROPERTY IMPORTED_LOCATION_RELEASE ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libminizip.lib)
#	        set_property(TARGET minizip PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libminizip.lib)
#	        set_property(TARGET minizip PROPERTY IMPORTED_LOCATION_DEBUG ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libminizip_d.lib)
#	        set(INCLUDES ${INCLUDES} ${LIB_ZLIB_ROOT_DIR}/include)
#	        set(LIBS ${LIBS} minizip)
#	    endif(ZLIB_MINI_LIB)
	    find_library(ZLIB_LIB NAMES zlibstatic PATHS ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
	    if(ZLIB_LIB)
	        add_library(zlib STATIC IMPORTED)
	        set_property(TARGET zlib PROPERTY IMPORTED_LOCATION_RELEASE ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/zlibstatic.lib)
	        set_property(TARGET zlib PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/zlibstatic.lib)
	        set_property(TARGET zlib PROPERTY IMPORTED_LOCATION_DEBUG ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/zlibstaticd.lib)
	        set(INCLUDES ${INCLUDES} ${LIB_ZLIB_ROOT_DIR}/include)
	        set(LIBS ${LIBS} zlib)
	    endif(ZLIB_LIB)
    else(WIN32)
	    find_library(ZLIB NAMES z PATHS ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT})
	    if(ZLIB)
	        add_library(libz SHARED IMPORTED)
	        set_property(TARGET libz PROPERTY IMPORTED_LOCATION ${LIB_ZLIB_ROOT_DIR}/lib/${CMAKE_SYSTEM_NAME}/${ARCH_HINT}/libz.so)
	        set(INCLUDES ${INCLUDES} ${LIB_ZLIB_ROOT_DIR}/include)
	        set(LIBS ${LIBS} libz)
	    else(ZLIB)
	        message(FATAL_ERROR "zlib not available, add it with -DLIB_ZLIB_ROOT_DIR =[path to zlib]") 
	    endif(ZLIB)
    endif(WIN32)
else(LIB_ZLIB_ROOT_DIR)  
    message(WARNING "zlib not available, add it with -DLIB_ZLIB_ROOT_DIR=[path to zlib]") 
    message(STATUS "trying to use zlib installed on system")
    # try to find the zlib library
    unset(ZLIB_FOUND CACHE)
    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        set(INCLUDES ${INCLUDES} ${ZLIB_INCLUDE_DIRS})
        set(LIBS ${LIBS} z)
        message(STATUS ${LIBS})
        message(STATUS ${ZLIB_INCLUDE_DIRS})
    else(ZLIB_FOUND)
        message(FATAL_ERROR "zlib not available, install it or add it with -DLIB_ZLIB_ROOT_DIR =[path to zlib]!")
    endif(ZLIB_FOUND)
endif(LIB_ZLIB_ROOT_DIR)

################################################################################
### option for enabling the https interface as builtin or shared library
################################################################################

# TODO: check this option on windows!!
set(ENABLE_HTTPS "Module" CACHE STRING "Select to enable building the https interface as builtin or shared module")
set_property(CACHE ENABLE_HTTPS PROPERTY STRINGS "Builtin" "Module")

if(ENABLE_HTTPS STREQUAL Module)
    set(ENABLE_HTTPS Module)
    set(ENABLE_HTTPS_MODULE ON)
else(ENABLE_HTTPS STREQUAL Module)
    if(ENABLE_HTTPS STREQUAL Builtin)
        set(ENABLE_HTTPS Builtin)
        set(ENABLE_HTTPS_MODULE OFF)
     else(ENABLE_HTTPS STREQUAL Builtin)
        set(ENABLE_HTTPS OFF)
        set(ENABLE_HTTPS_MODULE OFF)
     endif(ENABLE_HTTPS STREQUAL Builtin)
endif(ENABLE_HTTPS STREQUAL Module)

message(STATUS "building of https interface is [${ENABLE_HTTPS}]")
message(STATUS "to enable/disable building of https interface add -DENABLE_HTTPS={Module/Builtin}")

################################################################################
#########################  ADVANCE USER OPTIONS ################################
################################################################################

if(NOT WIN32) # linux only

################################################################################
### set use bison
################################################################################

option(ENABLE_USE_BISON "enable bison generated RuleParser [default=OFF]" OFF)
mark_as_advanced(ENABLE_USE_BISON)
# check input for this option
if(ENABLE_USE_BISON STREQUAL ON)
    set(ENABLE_USE_BISON ON)
else()
    set(ENABLE_USE_BISON OFF)  
endif()
message(STATUS "bison generated RuleParser is [${ENABLE_USE_BISON}]")
message(STATUS "to enable/disable bison generated RuleParser add -DENABLE_USE_BISON={ON|OFF}")

################################################################################
### set use flex
################################################################################

option(ENABLE_USE_FLEX "enable flex gererated RuleScanner [default=OFF]" OFF)
mark_as_advanced(ENABLE_USE_FLEX)
# check input for this option
if(ENABLE_USE_FLEX STREQUAL ON)
    set(ENABLE_USE_FLEX ON)
else()
    set(ENABLE_USE_FLEX OFF)  
endif()
message(STATUS "flex gererated RuleScanner is [${ENABLE_USE_FLEX}]")
message(STATUS "to enable/disable flex gererated RuleScanner add -DENABLE_USE_FLEX={ON|OFF}")

################################################################################
### set use gperf
################################################################################

option(ENABLE_USE_GPERF "enable gperf generated PaloCommands [default=OFF]" OFF)
mark_as_advanced(ENABLE_USE_GPERF)
# check input for this option
if(ENABLE_USE_GPERF STREQUAL ON)
    set(ENABLE_USE_GPERF ON)
else()
    set(ENABLE_USE_GPERF OFF)  
endif()
message(STATUS "gperf generated PaloCommands is [${ENABLE_USE_GPERF}]")
message(STATUS "to enable/disable gperf generated PaloCommands add -DENABLE_USE_GPERF={ON|OFF}")

################################################################################
### set verbose build
################################################################################

set(ENABLE_VERBOSE ON CACHE BOOL "set verbose build")

if(ENABLE_VERBOSE STREQUAL ON)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    set(CUDA_VERBOSE_BUILD ON)
else(ENABLE_VERBOSE STREQUAL ON)
    set(CMAKE_VERBOSE_MAKEFILE OFF)
    set(CUDA_VERBOSE_BUILD OFF)
endif(ENABLE_VERBOSE STREQUAL ON)

message(STATUS "enable verbose make files is [${CMAKE_VERBOSE_MAKEFILE}]")
message(STATUS "to enable/disable verbose build add -DENABLE_VERBOSE={ON/OFF}")

endif(NOT WIN32)
################################################################################
### option for enable trace
################################################################################

option(ENABLE_TRACE_OPTION "enable support for more trace output [default=OFF]" OFF)
mark_as_advanced(ENABLE_TRACE_OPTION)
# check input for this option
if(ENABLE_TRACE_OPTION STREQUAL ON)
    set(ENABLE_TRACE_OPTION ON)
else()
    set(ENABLE_TRACE_OPTION OFF)  
endif()
message(STATUS "support for extended trace output is [${ENABLE_TRACE_OPTION}]")
message(STATUS "to enable/disable support for extended trace output add -DENABLE_TRACE_OPTION={ON|OFF}")

if(NOT WIN32)
################################################################################
### option for gnu profiler
################################################################################

option(ENABLE_PROFILE "add -pg for profiling [default=OFF]" OFF)
mark_as_advanced(ENABLE_PROFILE)

# check input for this option
if(ENABLE_PROFILE STREQUAL ON)
  check_c_compiler_flag(-pg PGNUC)
  check_cxx_compiler_flag(-pg PGNUCXX)
  if(PGNUC AND PGNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(LDFLAGS "${LDFLAGS} -pg")
    set(ENABLE_PROFILE ON)
  else()
    set(ENABLE_PROFILE OFF)
    message(WARNING "no support gnu -pg profiling!")
  endif()
else()
  set(ENABLE_PROFILE OFF) 
endif()
message(STATUS "gnu profiler is [${ENABLE_PROFILE}]")
message(STATUS "to enable/disable -pg for profiling add -DENABLE_PROFILE={ON|OFF}")

################################################################################
### enable profiler
################################################################################

#check needed function
check_function_exists(getrusage HAVE_GETRUSAGE)

option(ENABLE_TIME_PROFILER "enable time profiling support [default=OFF]" OFF)
mark_as_advanced(ENABLE_TIME_PROFILER)

# check input for this option
if(ENABLE_TIME_PROFILER STREQUAL ON AND HAVE_GETRUSAGE)
    set(ENABLE_TIME_PROFILER ON)
else()
    set(ENABLE_TIME_PROFILER OFF)
endif()

message(STATUS "time profiling enabled is [${ENABLE_TIME_PROFILER}]")
message(STATUS "to enable/disable time profiling support add -DENABLE_TIME_PROFILER={ON|OFF}")

################################################################################
### enable select
################################################################################

# check header poll.h
check_include_file(poll.h HAVE_POLL_H)

option(ENABLE_SELECT "force the use of select instead of poll [default=OFF]" OFF)
mark_as_advanced(ENABLE_SELECT)

# check input for this option
if(ENABLE_SELECT STREQUAL ON AND HAVE_POLL_H)
    set(ENABLE_SELECT ON)
else()
    set(ENABLE_SELECT OFF)
endif()

message(STATUS "force use of select is [${ENABLE_SELECT}]")
message(STATUS "to enable/disable force the use of select instead of poll add -DENABLE_SELECT={ON|OFF}")

################################################################################
### enable pipe communication between scheduler and dispatcher
################################################################################

option(ENABLE_PIPE_COMM "use pipes to communication between scheduler and dispatcher [default=OFF]" OFF)
mark_as_advanced(ENABLE_PIPE_COMM)

if(ENABLE_PIPE_COMM STREQUAL ON)
    set(ENABLE_PIPE_COMM ON)
else()
    set(ENABLE_PIPE_COMM OFF)
endif()

message(STATUS "use pipes is [${ENABLE_PIPE_COMM}]")
message(STATUS "to enable/disable use of pipes add -DENABLE_PIPE_COMM={ON|OFF}")

################################################################################
### enable unittests and doxygen
################################################################################

# TODO: enable unittests and doxygen

################################################################################
### checks for system characteristics
################################################################################

# check BIGENDIAN
test_big_endian(WORDS_BIGENDIAN)
IF (WORDS_BIGENDIAN)
    ADD_DEFINITIONS("-DPALO_BIG_ENDIAN")
#ELSE (WORDS_BIGENDIAN)
#    ADD_DEFINITIONS("-DPALO_LITTLE_ENDIAN")
ENDIF (WORDS_BIGENDIAN)

# TODO: FIXIT test for DIR_SEPARATOR_CHAR missing
set(SEP_CHAR "/")
set(DIR_SEPARATOR_CHAR "${SEP_CHAR}")
set(DIR_SEPARATOR_STR "${SEP_CHAR}")

################################################################################
### checks for header files
################################################################################

# check stdc headers
CHECK_HEADER_STDC()
check_include_files(signal.h HAVE_SIGNAL_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(regex.h HAVE_REGEX_H)

check_include_files(sys/types.h HAVE_SYS_TYPES_H)
if(NOT HAVE_SYS_TYPES_H)
    set(SIZE_T "unsigned int")
endif()

# check headers
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)

check_include_files(unistd.h HAVE_UNISTD_H)

################################################################################
### checks for typedefs, structures, and compiler characteristics
################################################################################

check_type_size(suseconds_t HAVE_SUSECONDS_T)
check_type_size(size_t SIZEOF_SIZE_T)
check_type_size(double SIZEOF_DOUBLE)
check_type_size(void* SIZEOF_VOIDP)

# TODO: fix this alignof tests
check_type_size(double ALIGNOF_DOUBLE)
check_type_size(void* ALIGNOF_VOIDP)

# TODO: check if this correct
# check overloading functions for size_t and uint32_t/uint64_t

check_cxx_source_compiles(
    "#include <stdint.h>
     #include <stddef.h>
     void emil (size_t a) {}
     void emil (uint32_t a) {}
     void emil (uint64_t a) {}"
    OVERLOAD_FUNCS_SIZE_T)
if(OVERLOAD_FUNCS_SIZE_T)
    message(STATUS "overloading functions for size_t and uint32_t/uint64_t is possible")
endif()

# overloading functions for size_t and long int
check_cxx_source_compiles(
    "#include <stdint.h>
     #include <stddef.h>
     void emil (size_t a) {}
     void emil (long int a) {}
     emil (1);"
    OVERLOAD_FUNCS_SIZE_T_LONG)
if(OVERLOAD_FUNCS_SIZE_T_LONG)
    message(STATUS "overloading functions for size_t and long int is possible")
endif()

################################################################################
### check whether CC expects -export-dynamic
################################################################################

# TODO: fix check for -export-dynamic
check_library_exists(dl dlopen "" HAVE_DLOPEN)

if(HAVE_DLOPEN)
    set(INCLUDES ${INCLUDES} ${dl_INCLUDE_DIRS})
    set(LIBS ${LIBS} dl)
    link_directories(${dl_LIBRARY_DIRS})
else(HAVE_DLOPEN)
    # use -export-dynamic
endif(HAVE_DLOPEN)

################################################################################
### checks for libraries
################################################################################

# TODO: this is obsolete
# socket
check_library_exists(socket socket "/lib;/usr/lib;/usr/local/lib;/usr/pkg/lib" HAVE_SOCKETLIB)
# nsl
check_library_exists(nsl gethostbyname "/lib;/usr/lib;/usr/local/lib;/usr/pkg/lib" HAVE_NSLLIB)

################################################################################
### checks for pthreads
################################################################################

# TODO: fix pthreads
find_package(Threads)
message(STATUS "threads lib  ${CMAKE_THREAD_LIBS_INIT}")
if(CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
else(CMAKE_USE_PTHREADS_INIT)
    message(FATAL_ERROR "please install the pthread library")
endif(CMAKE_USE_PTHREADS_INIT)

endif(NOT WIN32)

################################################################################
### set project information
################################################################################

# palo patch changed automatically
if(ARCH_HINT STREQUAL i386)
    set(PALO_PATCH 2)
    set(ARCH i386)
else(ARCH_HINT STREQUAL i386)
    if(ENABLE_GPU_SERVER)
        set(PALO_PATCH 5)
    else(ENABLE_GPU_SERVER)
        set(PALO_PATCH 4)
    endif(ENABLE_GPU_SERVER)
    set(ARCH amd64)
endif(ARCH_HINT STREQUAL i386)

if(PALO_PATCH STREQUAL 5)
    set(HTTPS_PATCH 4)
else(PALO_PATCH STREQUAL 5)
    set(HTTPS_PATCH ${PALO_PATCH})
endif(PALO_PATCH STREQUAL 5)

# set package
set(PACKAGE "palo")
set(PACKAGE_BUGREPORT "bugs@jedox.com")
set(PACKAGE_NAME "Jedox Palo")
set(PACKAGE_VERSION "${PALO_MAJOR}.${PALO_MINOR}.${PALO_PATCH}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE}")
set(PACKAGE_URL "www.jedox.com")

################################################################################
### find subversion client to set right build number
################################################################################

set(REVISION "0000")
if(EXISTS ${PROJECT_SOURCE_DIR}/.svn OR EXISTS ${PROJECT_SOURCE_DIR}/../../../.svn)
    find_package(Subversion)
    if(Subversion_FOUND)
        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Source)
        set(REVISION "${Source_WC_LAST_CHANGED_REV}")
        message(STATUS "Current revison number: ${REVISION}")
    else(Subversion_FOUND)
        message(WARNING "Please install subversion to get right revision number!")
        set(REVISION "0000")
    endif(Subversion_FOUND)
endif()
file(WRITE ${CMAKE_BINARY_DIR}/olap_revision.txt ${REVISION})

################################################################################
### update version and revison out of source
################################################################################

configure_file(${PROJECT_SOURCE_DIR}/Config/build.h.in.cmake ${CMAKE_BINARY_DIR}/Config/build.h)

# update resource files
if(WIN32)
    configure_file(${PROJECT_SOURCE_DIR}/Resource/palohttpslib.rc.cmake ${CMAKE_BINARY_DIR}/Resource/palohttpslib.rc)
    configure_file(${PROJECT_SOURCE_DIR}/Resource/paloserver.rc.cmake ${CMAKE_BINARY_DIR}/Resource/paloserver.rc)
    configure_file(${PROJECT_SOURCE_DIR}/Resource/paloserverlib.rc.cmake ${CMAKE_BINARY_DIR}/Resource/paloserverlib.rc)
endif(WIN32)

if(NOT WIN32)

################################################################################
### prepare includes and source directories
################################################################################

# if bison found generate Library/Parser/RuleParser.hpp and Library/Parser/RuleParser.cpp
if(ENABLE_USE_BISON)
    if(BISON_FOUND)
        message(STATUS "Use bison version ${BISON_VERSION} to genarate RuleParser ...")
        execute_process(COMMAND ${BISON_EXECUTABLE} -l -d -ra -S lalr1.cc -o ${PROJECT_SOURCE_DIR}/Library/Parser/RuleParser.cpp ${PROJECT_SOURCE_DIR}/Library/Parser/RuleParser.yy RESULT_VARIABLE BISON_OK ERROR_QUIET)
        set(BISON_RuleParser_OK FALSE)
        if(BISON_OK STREQUAL 0)
            set(BISON_RuleParser_OK TRUE)
        endif(BISON_OK STREQUAL 0)
        message(STATUS "the bison macro run successfully: ${BISON_RuleParser_OK}")
    endif(BISON_FOUND)
endif(ENABLE_USE_BISON)

# if flex found generate Library/Parser/RuleScanner.cpp
if(ENABLE_USE_FLEX)
    if(FLEX_FOUND)
        message(STATUS "Use flex version ${FLEX_VERSION} to generate RuleScanner ...")
        execute_process(COMMAND ${FLEX_EXECUTABLE} --nounistd -L -o ${PROJECT_SOURCE_DIR}/Library/Parser/RuleScanner.cpp ${PROJECT_SOURCE_DIR}/Library/Parser/RuleScanner.ll RESULT_VARIABLE FLEX_OK ERROR_QUIET)
        set(FLEX_RuleScanner_OK FALSE)
        if(FLEX_OK STREQUAL 0)
            set(FLEX_RuleScanner_OK TRUE)
        endif(FLEX_OK STREQUAL 0)
        message(STATUS "the flex macro run successfully: ${FLEX_RuleScanner_OK}")
    endif(FLEX_FOUND)
endif(ENABLE_USE_FLEX)

# if grep found genarate Library/PaloHttpServer/PaloCommands.h
if(ENABLE_USE_GPERF)
    if(GPERF_FOUND)
        message(STATUS "Use gperf to generate PaloCommands ...")
        execute_process(COMMAND ${GPERF_FOUND} -CGD -N PaloValue -K option -L C++ -t ${PROJECT_SOURCE_DIR}/Library/PaloHttpServer/PaloCommands.gperf OUTPUT_FILE ${PROJECT_SOURCE_DIR}/Library/PaloHttpServer/PaloCommands.h RESULT_VARIABLE GPERF_OK ERROR_QUIET)
        set(GPERF_PaloCommands_OK FALSE)
        if(GPERF_OK STREQUAL 0)
            set(GPERF_PaloCommands_OK TRUE)
        endif(GPERF_OK STREQUAL 0)
        message(STATUS "the gperf macro run successfully: ${GPERF_PaloCommands_DEFINED}")
    endif(GPERF_FOUND)
endif(ENABLE_USE_GPERF)

# create config.h file
configure_file(${PROJECT_SOURCE_DIR}/Config/config.h.in.cmake ${CMAKE_BINARY_DIR}/Config/config.h)

endif(NOT WIN32)

if(WIN32)
    # create config_win.h file
    configure_file(${PROJECT_SOURCE_DIR}/Config/config_win.h.in.cmake ${CMAKE_BINARY_DIR}/Config/config_win.h)
endif(WIN32)	
    
# add includes
include_directories(${INCLUDES})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/Library)
include_directories(${CMAKE_BINARY_DIR}/Config)

# set directory list for palo library
set(libpalo_DIR_LIST
    Library/Collections
    Library/Dispatcher
    Library/DumpHandler
    Library/Engine
    Library/Exceptions
    Library/HttpServer
    Library/InputOutput
    Library/InputOutput/zip
    Library/Lists
    Library/Logger
    Library/Olap
    Library/Options 
    Library/PaloBrowser
    Library/PaloDispatcher
    Library/PaloDocumentation
    Library/PaloHttpServer
    Library/PaloJobs
    Library/Parser
    Library/Scheduler
    Library/System
    Library/Thread
    Library/Timer
    Library/VirtualMachine
    Library/Worker
    Programs
)

if (NOT WIN32)
set(libpalo_DIR_LIST
	${libpalo_DIR_LIST}
    Library/Network
    Library/Network/HttpClient
    Library/Network/TcpDevice
)
endif(NOT WIN32)

CollectFilesForLib("${libpalo_DIR_LIST}" LIBPALO_FILES)

set(FILE_LIST_TO_EXCLUDE "")
# need only for windows build in polo.exe
file(GLOB_RECURSE EXCLUDE_FILES Programs/palorun.cpp)
set(FILE_LIST_TO_EXCLUDE ${FILE_LIST_TO_EXCLUDE} ${EXCLUDE_FILES})
list(REMOVE_ITEM LIBPALO_FILES ${FILE_LIST_TO_EXCLUDE})

# https interface
if(ENABLE_HTTPS STREQUAL Builtin)
    # add built-in https files
    set(LIBPALO_FILES ${LIBPALO_FILES}
        Library/HttpsServer/HttpsServer.cpp
        Library/HttpsServer/HttpsServerTask.cpp)         
else(ENABLE_HTTPS STREQUAL Builtin)
    if(ENABLE_HTTPS STREQUAL Module)
        set(module_DIR_LIST Library/HttpsServer)
        CollectFilesForLib("${module_DIR_LIST}" MODULE_FILES)
    endif(ENABLE_HTTPS STREQUAL Module)                
endif(ENABLE_HTTPS STREQUAL Builtin)

# license extension
if(ENABLE_LICENSE_EXT STREQUAL ON)
    set(licext_DIR_LIST Library/License)
    CollectFilesForLib("${licext_DIR_LIST}" LICEXT_FILES)
endif(ENABLE_LICENSE_EXT STREQUAL ON)

# olap gpu accelerator
if(ENABLE_GPU_SERVER STREQUAL ON)
    set(palogpu_DIR_LIST
       Library/EngineGpu
       Library/PaloGpuJobs
       Library/PaloGpu/GpuEngine
       Library/PaloGpu/GpuStorage
       Library/PaloGpu/KernelUtils
       Library/PaloGpu/palo_gpu_rules.h
       Library/PaloGpu/palo_gpu.h
    )
    CollectFilesForLib("${palogpu_DIR_LIST}" LIBPALOGPU_FILES)
    set(cu_KERNEL_UTILS Library/PaloGpu/KernelUtils)
    CollectCudaFilesInDir(${cu_KERNEL_UTILS} CU_KERNEL_UTILS_FILES)
    set(cu_KERNEL Library/GpuKernels)
    CollectCudaFilesInDir(${cu_KERNEL} CU_KERNEL_FILES)
    set(CU_FILES ${CU_KERNEL_UTILS_FILES} ${CU_KERNEL_FILES})
    cuda_compile(CUDA_FILES ${CU_FILES})
    set(LIBPALO_FILES ${LIBPALO_FILES} ${LIBPALOGPU_FILES} ${CUDA_FILES} ${CU_FILES})
    #set(LIBS ${LIBS} ${CUDA_LIBRARIES})
endif(ENABLE_GPU_SERVER STREQUAL ON)

if(WIN32)
    # Library/Engine/StorageCpu.cpp should be compiled with /fp:precise flag
    set_property(SOURCE Library/Engine/StorageCpu.cpp PROPERTY COMPILE_FLAGS /fp:precise)
    
    # add target paloserver.dll
    source_group(Resource FILES ${CMAKE_BINARY_DIR}/Resource/paloserverlib.rc ${CMAKE_BINARY_DIR}/Resource/resource.h)
    add_library(paloserver SHARED ${LIBPALO_FILES} palo.h ${CMAKE_BINARY_DIR}/Resource/paloserverlib.rc)
    target_link_libraries(paloserver ${LIBS})
    set_target_properties(paloserver PROPERTIES COMPILE_FLAGS "/DEXPORTING_SERVER /DIMPORTING_HTTPS" LINK_FLAGS "/INCREMENTAL:NO /NOLOGO /SUBSYSTEM:WINDOWS /DEBUG")
    
     if(ENABLE_HTTPS STREQUAL Module)
        # add target palohttps.dll
        source_group(Resource FILES ${CMAKE_BINARY_DIR}/Resource/palohttpslib.rc ${CMAKE_BINARY_DIR}/Resource/resource.h)
        set(MODULE_FILES palo.h Library/System/system-vs-net-2003.h ${CMAKE_BINARY_DIR}/Resource/palohttpslib.rc ${MODULE_FILES})
        add_library(palohttps SHARED ${MODULE_FILES})
        target_link_libraries(palohttps ${OPENSSL_LIBS} paloserver ws2_32.lib)
        set_target_properties(palohttps PROPERTIES COMPILE_FLAGS "/DIMPORTING_SERVER /DEXPORTING_HTTPS" LINK_FLAGS "/INCREMENTAL:NO /NOLOGO /SUBSYSTEM:WINDOWS /DEBUG")
    endif(ENABLE_HTTPS STREQUAL Module)
    
    if(ENABLE_LICENSE_EXT STREQUAL ON)
        # add target palolic.dll
        configure_file(${PROJECT_SOURCE_DIR}/Resource/paloliclib.rc.cmake ${CMAKE_BINARY_DIR}/Resource/paloliclib.rc)
        source_group(Resource FILES ${CMAKE_BINARY_DIR}/Resource/paloliclib.rc ${CMAKE_BINARY_DIR}/Resource/resource.h)
        set(LICEXT_FILES palo.h Library/System/system-vs-net-2003.h ${CMAKE_BINARY_DIR}/Resource/paloliclib.rc Library/InputOutput/FileUtils.cpp ${LICEXT_FILES})
        add_library(palolic SHARED ${LICEXT_FILES})
        target_link_libraries(palolic paloserver)
        set_target_properties(palolic PROPERTIES COMPILE_FLAGS "/DIMPORTING_SERVER /DEXPORTING_LIC" LINK_FLAGS "/INCREMENTAL:NO /NOLOGO /SUBSYSTEM:WINDOWS /DEBUG")
    endif(ENABLE_LICENSE_EXT STREQUAL ON)
    
    # add target palo.exe 
    source_group(Resource FILES ${CMAKE_BINARY_DIR}/Resource/paloserver.rc ${CMAKE_BINARY_DIR}/Resource/resource.h)
    set(EXE_FILES Programs/palorun.cpp Library/System/system-vs-net-2003.h palo.h ${CMAKE_BINARY_DIR}/Resource/paloserver.rc)
    add_executable(palo ${EXE_FILES})
    set_target_properties(palo PROPERTIES LINK_FLAGS "/INCREMENTAL:NO /NOLOGO /SUBSYSTEM:CONSOLE /DEBUG")
    add_dependencies(palo paloserver)
    add_dependencies(palo palohttps)
    if(ENABLE_LICENSE_EXT STREQUAL ON)
        add_dependencies(palo palolic)
    endif(ENABLE_LICENSE_EXT STREQUAL ON)
    
else(WIN32)
        
    if(ENABLE_HTTPS STREQUAL Module)
    
        add_custom_target(clean.https.palo COMMAND rm -f ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libhttps.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH} VERBATIM)
        add_library(https.palo MODULE ${MODULE_FILES})
        target_link_libraries(https.palo ${OPENSSL_LIBS})
        if(SSL_LIBRARY)
            set_target_properties(https.palo PROPERTIES LINK_FLAGS -Wl,-Bsymbolic VERSION ${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH} SOVERSION 0)
        else(SSL_LIBRARY)
            set_target_properties(https.palo PROPERTIES VERSION ${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH} SOVERSION 0)
        endif(SSL_LIBRARY)

        if(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
            add_dependencies(https.palo clean.https.palo)
            add_custom_target(debug.https.palo ALL COMMAND objcopy --only-keep-debug libhttps.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH} libhttps.palo.dbg
                                                   COMMAND objcopy --strip-debug --strip-unneeded libhttps.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH}
                                                   COMMAND objcopy --add-gnu-debuglink=libhttps.palo.dbg libhttps.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH}
                                                   WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} VERBATIM)
            add_dependencies(debug.https.palo https.palo)
        endif(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
        
    endif(ENABLE_HTTPS STREQUAL Module)
    
    if(ENABLE_LICENSE_EXT STREQUAL ON)
        add_custom_target(clean.lic.palo COMMAND rm -f ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/liblic.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH} VERBATIM)
        add_library(lic.palo MODULE ${LICEXT_FILES})
        target_link_libraries(lic.palo ${LIC_LIBS})
        set_target_properties(lic.palo PROPERTIES VERSION ${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH} SOVERSION 0)
        
        if(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
            add_dependencies(lic.palo clean.lic.palo)
            add_custom_target(debug.lic.palo ALL COMMAND objcopy --only-keep-debug liblic.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH} liblic.palo.dbg
                                                 COMMAND objcopy --strip-debug --strip-unneeded liblic.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH}
                                                 COMMAND objcopy --add-gnu-debuglink=liblic.palo.dbg liblic.palo.so.${PALO_MAJOR}.${PALO_MINOR}.${HTTPS_PATCH}
                                                 WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} VERBATIM)
            add_dependencies(debug.lic.palo lic.palo)
        endif(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
        
    endif(ENABLE_LICENSE_EXT STREQUAL ON)

    add_custom_target(clean.${PACKAGE} COMMAND rm -f ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PACKAGE} VERBATIM)
    add_executable(${PACKAGE} ${LIBPALO_FILES})
    target_link_libraries(${PACKAGE} ${LIBS})
    
    if(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
        add_dependencies(${PACKAGE} clean.${PACKAGE})
        add_custom_target(debug.${PACKAGE} ALL COMMAND objcopy --only-keep-debug ${PACKAGE} ${PACKAGE}.dbg
                                               COMMAND objcopy --strip-debug --strip-unneeded ${PACKAGE}
                                               COMMAND objcopy --add-gnu-debuglink=${PACKAGE}.dbg ${PACKAGE}
                                               WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} VERBATIM)
        add_dependencies(debug.${PACKAGE} ${PACKAGE})
    endif(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
        
endif(WIN32)

################################################################################
### enable doxygen to create documentation
################################################################################

find_package(Doxygen)

if(DOXYGEN_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in.cmake ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)

################################################################################
### summary
################################################################################

message(STATUS "==================== Summary =========================")
message(STATUS "Configuring: Current version is ${PACKAGE_VERSION}")
message(STATUS "Configuring: Current revison is ${REVISION}")
message(STATUS "Configuring: Current target system is ${CMAKE_SYSTEM}-${ARCH_HINT}")
message(STATUS "Configuring: Extended trace output is ${ENABLE_TRACE_OPTION}")

if(ENABLE_HTTPS STREQUAL OFF)
    message(STATUS "Configuring: https interface is disabled")   
else()
    if(ENABLE_HTTPS STREQUAL Module)
        message(STATUS "Configuring: https interface is compiled as shared library")
    else()
        message(STATUS "Configuring: https interface is compiled as built-in")
    endif()
endif()

if(ENABLE_LICENSE_EXT STREQUAL OFF)
    message(STATUS "Configuring: palolic extension is disabled")   
else()
    message(STATUS "Configuring: palolic extension compiled as shared library")
endif()

if(NOT WIN32)
message(STATUS "Configuring: Current project build type is: ${PROJECT_BUILD_TYPE}")
if(BISON_RuleParser_OK)
    message(STATUS "Configuring: BISON is available: ${BISON_EXECUTABLE}")
else()
    message(STATUS "Configuring: need a version of bison - changes to the .yy files will have no effect")
endif()

if(FLEX_RuleScanner_OK)
    message(STATUS "Configuring: LEX is available: ${FLEX_EXECUTABLE}")
else()
    message(STATUS "Configuring: need a version of flex - changes to the .ll files will have no effect")
endif()

if(GPERF_PaloCommands_OK)
    message(STATUS "Configuring: GPERF is available: ${GPERF_FOUND}")
else()
    message(STATUS "Configuring: need a version of gperf - changes to the .gperf files will have no effect")
endif()

if(ENABLE_TIME_PROFILER STREQUAL ON)
    message(STATUS "Configuring: time profiler is enabled")
else()
    message(STATUS "Configuring: time profiler is disabled")
endif()
if(PROJECT_BUILD_TYPE STREQUAL Release)
    message(STATUS "Configuring: C_FLAGS: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
    message(STATUS "Configuring: CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
else(PROJECT_BUILD_TYPE STREQUAL Release)
    if(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
        message(STATUS "Configuring: C_FLAGS: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
        message(STATUS "Configuring: CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    else(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
        message(STATUS "Configuring: C_FLAGS: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
        message(STATUS "Configuring: CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    endif(PROJECT_BUILD_TYPE STREQUAL RelWithDebInfo)
endif(PROJECT_BUILD_TYPE STREQUAL Release)

if(GPU_ENGINE STREQUAL ON)
    message(STATUS "Configuring: NVCC_FLAGS: ${CUDA_NVCC_FLAGS}")
endif()

message(STATUS "Configuring: LIBS: ${LIBS}")
message(STATUS "Configuring: OPENSSL_LIBS: ${OPENSSL_LIBS}")

if(ENABLE_LICENSE_EXT STREQUAL ON)
    message(STATUS "Configuring: LIC_LIBS: ${LIC_LIBS}")
endif()
message(STATUS "Configuring: verbose make files is: ${CMAKE_VERBOSE_MAKEFILE}")
endif(NOT WIN32)
message(STATUS "======================================================")

################################################################################
### CPack
################################################################################

if(WIN32)
    install(TARGETS palo paloserver palohttps
        RUNTIME DESTINATION ./
        LIBRARY DESTINATION ./
        #ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/archive
        )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/palo.pdb
                  ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/paloserver.pdb
                  ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/palohttps.pdb 
                  DESTINATION ./)
    # license extension
    if(ENABLE_LICENSE_EXT STREQUAL ON) 
          install(TARGETS palolic
              RUNTIME DESTINATION ./
              LIBRARY DESTINATION ./)
          install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/palolic.pdb DESTINATION ./)
    endif(ENABLE_LICENSE_EXT STREQUAL ON)
    
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
    set(CPACK_GENERATOR "ZIP")
else(WIN32)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/usr DESTINATION ./ USE_SOURCE_PERMISSIONS)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
    set(CPACK_GENERATOR "TGZ")
endif(WIN32)

set(CPACK_PACKAGE_NAME ${PACKAGE})
if(ENABLE_TRACE_OPTION)
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE}-${CMAKE_SYSTEM_NAME}-${ARCH}-${PACKAGE_VERSION}.${REVISION}.trace_on")
else(ENABLE_TRACE_OPTION)
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE}-${CMAKE_SYSTEM_NAME}-${ARCH}-${PACKAGE_VERSION}.${REVISION}")
endif(ENABLE_TRACE_OPTION)
#message(STATUS ${CPACK_PACKAGE_FILE_NAME})
set(CPACK_PACKAGE_VENDOR "Jedox AG")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PALO - OpenSource MOLAP (www.palo.net)")
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PALO_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PALO_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PALO_PATCH})
set(CPACK_PACKAGE_EXECUTABLES ${PACKAGE})
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")
#set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")

#deb package
set(CPACK_PACKAGE_CONTACT ${PACKAGE_BUGREPORT})
# TODO: add dependencies 
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${ARCH}")
set(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
set(CPACK_DEBIAN_PACKAGE_SECTION "Miscellaneous")

set(CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
#set(CPACK_RPM_PACKAGE_ARCHITECTURE  "${ARCH}")
set(CPACK_RPM_PACKAGE_REQUIRES  "")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PALO - OpenSource MOLAP (www.palo.net)")
set(CPACK_PACKAGE_DESCRIPTION "bla bla")
include(CPack)
