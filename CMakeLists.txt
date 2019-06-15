set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(cwb C)

# Release or Debug
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MACOSX_RPATH ON)

option(BUILD_SHARED_LIBS "Set to OFF to use static library" ON)

if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /permissive- /W4 /MP")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else()
	set(_FLAGS_COMMON "-Wall -Wextra -Wno-missing-field-initializers -Wno-deprecated -Wno-unused-parameter -fPIC")

	include(CheckCCompilerFlag)

	foreach(flag "-Wno-unused-result" "-flto")
		string(REGEX REPLACE "[^A-Za-z0-9]" "-" _flag ${flag})
		CHECK_C_COMPILER_FLAG(${flag} COMPILER_SUPPORTS_${_flag})
		if(COMPILER_SUPPORTS_${_flag})
			set(_FLAGS_COMMON "${_FLAGS_COMMON} ${flag}")
		endif()
	endforeach()
	if(COMPILER_SUPPORTS_flto)
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
	endif()

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_FLAGS_COMMON}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

	# Enable latest possible C standard
	foreach(flag "-std=c2x" "-std=c11" "-std=c1x" "-std=c99")
		string(REGEX REPLACE "[^a-z0-9]" "-" _flag ${flag})
		CHECK_C_COMPILER_FLAG(${flag} COMPILER_SUPPORTS_${_flag})
		if(COMPILER_SUPPORTS_${_flag})
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
			break()
		endif()
	endforeach()
endif()

include(GNUInstallDirs)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

if(VCPKG_TOOLCHAIN)
	find_package(unofficial-glib CONFIG REQUIRED)
	set(GLIB_LIBRARIES unofficial::glib::gio unofficial::glib::glib unofficial::glib::gmodule unofficial::glib::gobject)
else()
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLIB REQUIRED glib-2.0)
	include_directories(${GLIB_INCLUDE_DIRS})
	link_directories(${GLIB_LIBRARY_DIRS})
endif()

# PCRE
find_library(PCRE_LIB pcre)
find_path(PCRE_INCLUDE pcre.h)
include_directories(${PCRE_INCLUDE})

# getopt
find_path(GETOPT_INCLUDE getopt.h)
include_directories(${GETOPT_INCLUDE})
if(VCPKG_TOOLCHAIN)
	find_library(GETOPT_LIB NAMES getopt)
else()
	set(GETOPT_LIB)
endif()

# dirent
find_path(DIRENT_INCLUDE dirent.h)
include_directories(${DIRENT_INCLUDE})

# gettimeofday
if(VCPKG_TOOLCHAIN)
	find_library(GTOD_LIB NAMES gettimeofday)
else()
	set(GTOD_LIB)
endif()

add_definitions(-DVERSION="3.4.16")

if(MSVC)
	# Pretend to be MinGW because lots of code already checks for that, which will work for MSVC
	add_definitions(-D__MINGW__ -DYY_NO_UNISTD_H -DEMULATE_SETENV)
endif()

if(WIN32)
	add_definitions(-D_SECURE_SCL=0 -D_ITERATOR_DEBUG_LEVEL=0 -D_CRT_SECURE_NO_DEPRECATE -DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DNOMINMAX)
	include_directories("include")
else()
	add_definitions(-D_POSIX_C_SOURCE=200112 -D_GNU_SOURCE)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cl)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cqp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)
