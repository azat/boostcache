
#
# Cmake rules for boostcache project
#

cmake_minimum_required(VERSION 2.6)
project(BoostCache CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include(AddCompilerFlags)
include(CheckCXXCompilerFlag)
include(FindLibrariesOrFatalError)
include(AddFilesGlobRecursiveToList)
include(GitVersion)

message(STATUS "Making ${PROJECT_NAME} on ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")

find_package(Boost REQUIRED COMPONENTS program_options system log)

FindLibrariesOrFatalError("boost_program_options boost_system boost_log")

# The version number.
set(BoostCache_VERSION_MAJOR 0)
set(BoostCache_VERSION_MINOR 10)
GitVersion(BoostCache)

# Directories
set(BOOSTCACHE_ROOT_DIR "${CMAKE_SOURCE_DIR}")
set(BOOSTCACHE_BINARY_DIR "${BOOSTCACHE_ROOT_DIR}/build")
set(BOOSTCACHE_BUILD_DIR "${BOOSTCACHE_ROOT_DIR}/build")
set(BOOSTCACHE_SOURCE_DIR "${BOOSTCACHE_ROOT_DIR}/src")
set(BOOSTCACHE_LIBRARY_DIR "${BOOSTCACHE_ROOT_DIR}/lib")
set(BOOSTCACHE_EXECUTABLE_OUTPUT_PATH ${BOOSTCACHE_BUILD_DIR} CACHE PATH "Build directory" FORCE)
set(BOOSTCACHE_LIBRARY_OUTPUT_PATH ${BOOSTCACHE_BUILD_DIR} CACHE PATH "Build directory" FORCE)

# Include paths
include_directories(${BOOSTCACHE_SOURCE_DIR})
include_directories(${BOOSTCACHE_BINARY_DIR})

AddCompilerFlags(FAIL_ON_ERROR FLAGS -Wall -Wextra -Werror -g -rdynamic LANGUAGES C CXX)
AddCompilerFlags(FAIL_ON_ERROR FLAGS -std=c++11 LANGUAGES CXX)
add_definitions(-D_REENTRANT -DTHREAD_SAFE -D_GNU_SOURCE -DDEBUG)

# boostcache sources
AddFilesGlobRecursiveToList(BOOSTCACHE_SOURCES "${BOOSTCACHE_SOURCE_DIR}/client/*.cpp")
AddFilesGlobRecursiveToList(BOOSTCACHE_SOURCES "${BOOSTCACHE_SOURCE_DIR}/kernel/*.cpp")
AddFilesGlobRecursiveToList(BOOSTCACHE_SOURCES "${BOOSTCACHE_SOURCE_DIR}/db/*.cpp")
AddFilesGlobRecursiveToList(BOOSTCACHE_SOURCES "${BOOSTCACHE_SOURCE_DIR}/kernel/net/*.cpp")
AddFilesGlobRecursiveToList(BOOSTCACHE_SOURCES "${BOOSTCACHE_SOURCE_DIR}/util/*.cpp")
AddFilesGlobRecursiveToList(BOOSTCACHE_SOURCES "${BOOSTCACHE_SOURCE_DIR}/wrapper/*.cpp")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
    "${BOOSTCACHE_SOURCE_DIR}/config.h.in"
    "${BOOSTCACHE_BINARY_DIR}/config.h"
)

# boostcache server
file(GLOB_RECURSE BOOSTCACHE_SERVER_SOURCES "${BOOSTCACHE_SOURCE_DIR}/server/*.cpp")
list(APPEND BOOSTCACHE_SERVER_SOURCES ${BOOSTCACHE_SOURCES})
add_executable(boostcached ${BOOSTCACHE_SERVER_SOURCES})
target_link_libraries(boostcached ${LIBS})
