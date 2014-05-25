
#
# Cmake rules for boostcache project
#

cmake_minimum_required(VERSION 2.8.11)
project(BoostCache CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include(ExternalProject)
include(AddCompilerFlags)
include(GitVersion)
include(FindLibrary)
include(TryCompileFromSource)
include(Configure)

message(STATUS "Making ${PROJECT_NAME} on ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")

find_package(Boost REQUIRED COMPONENTS program_options system log thread)

# v8
FindLibrary(FAIL_ON_ERROR NAMES v8)
TryCompileFromSource(
    HAVE_V8_FUNCTIONCALLBACKINFO
    "v8 with FunctionCallbackInfo instead of Arguments"
"
#include <v8.h>
int main()
{
    (void)sizeof(v8::FunctionCallbackInfo<v8::Value>);
    return 0;
};
"
)
TryCompileFromSource(
    HAVE_V8_WITH_MOST_CONSTRUCTORS_ISOLATE
    "v8 most of constructors have Isolate* argument, instead of getting current"
"
#include <v8.h>
int main()
{
    v8::HandleScope scope(v8::Isolate::GetCurrent());
    (void)scope;
    return 0;
};
"
    LINK_LIBRARIES ${LIBS}
)

# The version number.
GitVersion(BoostCache)
GitVersionToMajorMinor(BoostCache ${BoostCache_GIT_VERSION})

# Directories
set(BOOSTCACHE_ROOT_DIR "${CMAKE_SOURCE_DIR}")
set(BOOSTCACHE_BUILD_DIR "${BOOSTCACHE_ROOT_DIR}/build"
    CACHE STRING "Set build dir")
set(BOOSTCACHE_SOURCE_DIR "${BOOSTCACHE_ROOT_DIR}/src")
set(BOOSTCACHE_UTILS_DIR "${BOOSTCACHE_ROOT_DIR}/utils")
set(BOOSTCACHE_LIBRARY_DIR "${BOOSTCACHE_ROOT_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH ${BOOSTCACHE_BUILD_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${BOOSTCACHE_BUILD_DIR} CACHE PATH "Build directory" FORCE)

set(BOOSTCACHE_HEADER_CONFIG "${BOOSTCACHE_BUILD_DIR}/config.h.in")

# Include paths
include_directories(${BOOSTCACHE_SOURCE_DIR})
include_directories(${BOOSTCACHE_BUILD_DIR})

# Common compiler flags
set(CMAKE_EXE_LINKER_FLAGS -pthread)
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

macro(SetupLTOForClang)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        AddCompilerFlags(FAIL_ON_ERROR FLAGS
                         "-B${BOOSTCACHE_ROOT_DIR}/utils/toolchain"
                         LANGUAGES C CXX)
    endif()
endmacro()
SetupLTOForClang()

AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE PROFILE FLAGS
                 -g3 --coverage
                 OVERWRITE --coverage
                 LANGUAGES C CXX)
AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE DEBUG FLAGS
                 -g3
                 LANGUAGES C CXX)
AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE RELEASE FLAGS
                 -O3 -g3 -flto
                 OVERWRITE -flto
                 LANGUAGES C CXX)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-fwhole-program")
AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE DISTRIBUTION FLAGS
                 -O3 -flto -march=native
                 LANGUAGES C CXX)
# Sanitizers
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE SANITIZETHREAD
                     FLAGS -g3 -fsanitize=thread
                     OVERWRITE -fsanitize=thread
                     LANGUAGES C CXX)
    AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE SANITIZEADDRESS
                     FLAGS "-g3 -fsanitize=address"
                     OVERWRITE -fsanitize=address
                     LANGUAGES C CXX)
    AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE SANITIZEMEMORY
                     FLAGS -g3 -fsanitize=memory -fno-omit-frame-pointer
                     OVERWRITE -fsanitize=memory -fsanitize-memory-track-origins
                     LANGUAGES C CXX)
else()
    AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE SANITIZETHREAD
                     FLAGS -g3 "-fsanitize=thread -fPIC"
                     OVERWRITE "-fsanitize=thread -pie"
                     LANGUAGES C CXX)
    AddCompilerFlags(FAIL_ON_ERROR BUILD_TYPE SANITIZEADDRESS
                     FLAGS -g3 -fsanitize=address
                     OVERWRITE -fsanitize=address
                     LANGUAGES C CXX)
endif()

add_definitions(-D_REENTRANT -DTHREAD_SAFE -D_GNU_SOURCE -DDEBUG)
AddCompilerFlags(FAIL_ON_ERROR FLAGS
                 "-include ${BOOSTCACHE_SOURCE_DIR}/util/compilercompat.h"
                 LANGUAGES C CXX)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
# Common flags
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
message(STATUS " C flags  : ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
message(STATUS " CXX flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")

# boostcache sources
list(APPEND BOOSTCACHE_SOURCES
    "${BOOSTCACHE_SOURCE_DIR}/db/avltree.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/db/hashtable.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/db/interface.cpp"

    "${BOOSTCACHE_SOURCE_DIR}/kernel/commandhandler.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/kernel/commands.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/kernel/net/commandserver.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/kernel/net/ioservicepool.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/kernel/net/session.cpp"

    "${BOOSTCACHE_SOURCE_DIR}/server/boostcached.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/server/jsvm.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/server/options.cpp"

    "${BOOSTCACHE_SOURCE_DIR}/util/log.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/util/options.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/util/stacktrace.cpp"
    "${BOOSTCACHE_SOURCE_DIR}/util/version.cpp"
)

# Pass defines to project
macro(ConfigureBoostcache)
    file(WRITE "${BOOSTCACHE_HEADER_CONFIG}" "// Generated with cmake\n\n")

    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)

    set(BOOSTCACHE_VERSION_FLOAT "${BoostCache_VERSION_MAJOR}.${BoostCache_VERSION_MINOR}")
    set(BOOSTCACHE_VERSION_GIT
        "${BoostCache_GIT_VERSION}-${BoostCache_GIT_BRANCH}-${CMAKE_BUILD_TYPE_UPPER}")
    set(BOOSTCACHE_NAME "${CMAKE_PROJECT_NAME}")

    configure(STRING_VAL FILE "${BOOSTCACHE_HEADER_CONFIG}" DEFINES
        BOOSTCACHE_VERSION_GIT
        BOOSTCACHE_NAME
    )
    configure(VAL FILE "${BOOSTCACHE_HEADER_CONFIG}" DEFINES
        BOOSTCACHE_VERSION_FLOAT
    )
    configure(FILE "${BOOSTCACHE_HEADER_CONFIG}" DEFINES
        HAVE_V8_FUNCTIONCALLBACKINFO
        HAVE_V8_WITH_MOST_CONSTRUCTORS_ISOLATE
    )

    configure_file(
        "${BOOSTCACHE_HEADER_CONFIG}"
        "${BOOSTCACHE_BUILD_DIR}/config.h"
    )
endmacro()
ConfigureBoostcache()

# boostcache server
file(GLOB_RECURSE BOOSTCACHE_SERVER_SOURCES "${BOOSTCACHE_SOURCE_DIR}/server/*.cpp")
list(APPEND BOOSTCACHE_SERVER_SOURCES ${BOOSTCACHE_SOURCES})
add_executable(boostcached ${BOOSTCACHE_SERVER_SOURCES})
target_link_libraries(boostcached ${Boost_LIBRARIES} ${LIBS})
install(TARGETS boostcached RUNTIME DESTINATION bin)
install(PROGRAMS "${BOOSTCACHE_UTILS_DIR}/boostcache" DESTINATION bin)
install(FILES "${BOOSTCACHE_UTILS_DIR}/bash_completion"
        RENAME "boostcache"
        DESTINATION "/etc/bash_completion.d")

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
install(PROGRAMS "${BENCHMARK_SRC_DIR}/bc-benchmark" DESTINATION bin)

macro(AddCustomTarget name)
    add_custom_target(${name} COMMAND ${ARGN})
endmacro()
AddCustomTarget(runtests
                ${BOOSTCACHE_UTILS_DIR}/run_tests.sh
                ${BOOSTCACHE_BUILD_DIR}/boostcached
                ${BOOSTCACHE_BUILD_DIR}/boostcached.sock
)
AddCustomTarget(runsanitizers ${BOOSTCACHE_UTILS_DIR}/sanitizers.sh)
AddCustomTarget(runbenchmarks
                ${BOOSTCACHE_UTILS_DIR}/run_benchmarks.sh
                ${BOOSTCACHE_BUILD_DIR}/boostcached
                ${BOOSTCACHE_BUILD_DIR}/boostcached.sock
)
AddCustomTarget(buildplots
                ${BOOSTCACHE_UTILS_DIR}/build_graphs.sh
                -b ${BOOSTCACHE_BUILD_DIR}/boostcached
                -s ${BOOSTCACHE_BUILD_DIR}/boostcached.sock
)
AddCustomTarget(buildplots-png
                ${BOOSTCACHE_UTILS_DIR}/build_graphs.sh -w2 -f png
                -b ${BOOSTCACHE_BUILD_DIR}/boostcached
                -s ${BOOSTCACHE_BUILD_DIR}/boostcached.sock
)
AddCustomTarget(buildplots-dumb
                ${BOOSTCACHE_UTILS_DIR}/build_graphs.sh -w2 -f dumb
                -b ${BOOSTCACHE_BUILD_DIR}/boostcached
                -s ${BOOSTCACHE_BUILD_DIR}/boostcached.sock
)

macro(GeneratePackage)
    include(GenerateDebianControl)
    set(CPACK_PACKAGE_NAME "boostcache-git")
    file(READ "${BOOSTCACHE_ROOT_DIR}/README.md" README)
    GenerateDebianControl(${README} DEBIAN_CONTROL)
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${DEBIAN_CONTROL}")
    set(CPACK_PACKAGE_VERSION "${BOOSTCACHE_VERSION_GIT}")
    GitToDebVersion(CPACK_PACKAGE_VERSION)
    set(CPACK_PACKAGE_MAINTAINER "a3at.mail@gmail.com")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_MAINTAINER}")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS    "libc6 (>= 2.13),"
                                        "libgcc1 (>= 4.7-2),"
                                        "libstdc++6 (>= 4.8),"
                                        "libv8-dev (>=3.14),"
                                        "libboost-program-options1.54-dev,"
                                        "libboost-system1.54-dev,"
                                        "libboost-log1.54-dev,"
                                        "libboost-thread1.54-dev")
    JoinArguments(CPACK_DEBIAN_PACKAGE_DEPENDS)
    # TODO: strip in productin
    set(CPACK_STRIP_FILES NO)
    set(CPACK_PACKAGE_HOMEPAGE "https://github.com/azat/boostcache")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE}")
    set(CPACK_GENERATOR "DEB")
    include(CPack)
endmacro(GeneratePackage)
GeneratePackage()

