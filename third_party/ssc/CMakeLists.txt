#####################################################################################################################
#
# CMake and System Settings
#
#####################################################################################################################

cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

option(SAM_SKIP_TOOLS "Skips the sdktool and tcsconsole builds" OFF)

option(SAM_SKIP_TESTS "Skips building tests" OFF)

option(SAMAPI_EXPORT "Export of ssc binaries to the SAM_api directory; for Unix, compile ssc libraries for SAM_api" ON)

#
# If project isn't system_advisor_model and SAM_SKIP_TOOLS=1,
#   environment vars LK_LIB and LKD_LIB can be used to specify where to find those libraries
#

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
endif()

if (UNIX AND NOT CMAKE_C_COMPILER)
	set(CMAKE_C_COMPILER gcc)
	set(CMAKE_CXX_COMPILER g++)
endif()
set(CMAKE_CXX_STANDARD 11)

if ( NOT APPLE)
    set(CURL_DIR build_resources/libcurl_ssl_x64)
endif()

Project(sam_simulation_core VERSION 1.0.0)


#####################################################################################################################
#
# Compile Options per Platform
#
#####################################################################################################################
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 11)

function(set_no_warnings target)
    if(MSVC)
      target_compile_options(${target} PRIVATE /W0)
    else()
      target_compile_options(${target} PRIVATE -w)
    endif()
endfunction()

function(set_disabled_warnings target)
    if(MSVC)
      target_compile_options(${target} PRIVATE /W4 /w44191 /w44242  /w44266 /w44302 /w44388 /w44826 /w44905 /w44906 /w44928)
    else()
      target_compile_options(${target} PRIVATE -Wall -Wno-unknown-pragmas -Wno-deprecated)
      if(APPLE)
        target_compile_options(${target} PRIVATE -Wno-unused-private-field -Wno-unused-private-field -Wno-ignored-attributes)
      endif()
    endif()
endfunction()

function(set_default_compile_options target)
  target_compile_definitions(${target} PRIVATE __64BIT__)
  if(MSVC)
    target_compile_options(${target} PRIVATE /bigobj /MP)
    target_compile_definitions(${target} PRIVATE  __WINDOWS__ _WINDOWS _CRT_SECURE_NO_WARNINGS LPWINAPP)
    target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:_DEBUG>)
  else()
    target_compile_features(${target} PRIVATE cxx_std_11)
    if (APPLE)
      target_compile_options(${target} PRIVATE -fno-common)
      target_compile_definitions(${target} PRIVATE WX_PRECOMP __MACOSX__)
    else()
      target_compile_definitions(${target} PRIVATE __UNIX__)
    endif()
    target_compile_definitions(${target} PRIVATE $<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>)
  endif()
endfunction()

function(set_tools_compile_options target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W3)
    target_compile_definitions(${target} PRIVATE
      LK_USE_WXWIDGETS
      _CRT_SECURE_NO_DEPRECATE=1
      _CRT_NON_CONFORMING_SWPRINTFS=1
      _SCL_SECURE_NO_WARNINGS=1
      __WXMSW__
      _UNICODE
      _NOPCH
    )
  else()
    if (APPLE)
      target_compile_definitions(${target} PRIVATE __WXOSX__)
    endif()
    target_compile_features(${target} PRIVATE cxx_std_11)
    target_compile_options(${target} PRIVATE -Wall -Wno-deprecated -Wno-unknown-pragmas -Werror -Wno-overloaded-virtual -fno-common)
    target_compile_definitions(${target} PRIVATE LK_USE_WXWIDGETS)
  endif()
endfunction()

function(set_additional_compile_options target options)
  target_compile_options(${target} PRIVATE ${options})
endfunction()

#####################################################################################################################
#
# Project Settings
#
#####################################################################################################################

# turn off examples, tests and install for jsoncpp
set(JSONCPP_WITH_EXAMPLE 0)
set(JSONCPP_WITH_TESTS 0)

# Skip install() commands for jsoncpp
add_subdirectory(jsoncpp EXCLUDE_FROM_ALL)

add_subdirectory(splinter)
add_subdirectory(shared)
#add_subdirectory(nlopt)
#add_subdirectory(lpsolve)
#add_subdirectory(solarpilot)
#add_subdirectory(tcs)
add_subdirectory(ssc)

if (NOT SAM_SKIP_TOOLS)
	add_subdirectory(sdktool)
	add_subdirectory(tcsconsole)
endif()

if (NOT SAM_SKIP_TESTS)
	add_subdirectory(test)
endif()
