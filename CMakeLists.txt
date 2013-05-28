
#
# Cmake rules for boostcache project
#

cmake_minimum_required(VERSION 2.6)
project(BoostCache CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include(ExternalProject)
include(AddCompilerFlags)
include(CheckCXXCompilerFlag)
include(FindLibrary)
include(AddFilesGlobRecursiveToList)
include(GitVersion)

message(STATUS "Making ${PROJECT_NAME} on ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")

find_package(Boost REQUIRED COMPONENTS program_options system log thread)

FindLibrary(FAIL_ON_ERROR NAMES
            boost_program_options
            boost_system
            boost_log
            boost_thread)

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

# Common compiler flags
AddCompilerFlags(FAIL_ON_ERROR FLAGS
                 -Wall -Wextra -Werror
                 LANGUAGES C CXX)
AddCompilerFlags(FAIL_ON_ERROR FLAGS
                 -std=c++11
                 LANGUAGES CXX)
AddCompilerFlags(FLAGS -rdynamic
                 LANGUAGES C CXX)
# Per BUILD_TYPE
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Profile" "Debug" "Release" "Distribution")
endif()

AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE PROFILE FLAGS
                 -g3 --coverage
                 LANGUAGES C CXX)
AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE DEBUG FLAGS
                 -g3
                 LANGUAGES C CXX)
AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE RELEASE FLAGS
                 -ofast -g3
                 LANGUAGES C CXX)
AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE DISTRIBUTION FLAGS
                 -ofast -march=native
                 LANGUAGES C CXX)
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

set(BENCHMARK_TRASH_DIR "${BOOSTCACHE_BUILD_DIR}")
set(BENCHMARK_SRC_DIR "${BOOSTCACHE_SOURCE_DIR}/benchmark")
ExternalProject_Add(benchmark
    # PREFIX "${BENCHMARK_SRC_DIR}"
    TMP_DIR "${BENCHMARK_TRASH_DIR}"
    STAMP_DIR "${BENCHMARK_TRASH_DIR}"
    # Download step
    # Instead download do
    # git submodule init
    # git submodule update
    DOWNLOAD_COMMAND ""
    # GIT_REPOSITORY ${BENCHMARK_REPO}
    # Update
    # Do not fetch
    UPDATE_COMMAND ""
    # Configure
    SOURCE_DIR "${BENCHMARK_SRC_DIR}"
    CONFIGURE_COMMAND ""
    # Build step
    BINARY_DIR "${BENCHMARK_SRC_DIR}"
    BUILD_COMMAND make
    # Install
    INSTALL_DIR "${BENCHMARK_SRC_DIR}"
    INSTALL_COMMAND ""
)
