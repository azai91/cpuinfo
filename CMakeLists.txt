CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12 FATAL_ERROR)

INCLUDE(GNUInstallDirs)

# ---[ Project and semantic versioning.
PROJECT(cpuinfo C CXX)

# ---[ Options.
SET(CPUINFO_LIBRARY_TYPE "default" CACHE STRING "Type of cpuinfo library (shared, static, or default) to build")
SET_PROPERTY(CACHE CPUINFO_LIBRARY_TYPE PROPERTY STRINGS default static shared)
SET(CPUINFO_RUNTIME_TYPE "default" CACHE STRING "Type of runtime library (shared, static, or default) to use")
SET_PROPERTY(CACHE CPUINFO_RUNTIME_TYPE PROPERTY STRINGS default static shared)
OPTION(CPUINFO_LOG_TO_STDIO "Log errors, warnings, and information to stdout/stderr" ON)
OPTION(CPUINFO_BUILD_TOOLS "Build command-line tools" ON)
OPTION(CPUINFO_BUILD_UNIT_TESTS "Build cpuinfo unit tests" ON)
OPTION(CPUINFO_BUILD_MOCK_TESTS "Build cpuinfo mock tests" ON)
OPTION(CPUINFO_BUILD_BENCHMARKS "Build cpuinfo micro-benchmarks" ON)

#set(CMAKE_SYSTEM_NAME Linux)

# ---[ CMake options
IF(CPUINFO_BUILD_UNIT_TESTS OR CPUINFO_BUILD_MOCK_TESTS)
  ENABLE_TESTING()
ENDIF()

MACRO(CPUINFO_TARGET_ENABLE_C99 target)
  IF(${CMAKE_VERSION} VERSION_LESS "3.1")
    IF(NOT MSVC)
      TARGET_COMPILE_OPTIONS(${target} PRIVATE -std=c99)
    ENDIF()
  ELSE()
    SET_TARGET_PROPERTIES(${target} PROPERTIES
      C_STANDARD 99
      C_EXTENSIONS NO)
  ENDIF()
ENDMACRO()

MACRO(CPUINFO_TARGET_ENABLE_CXX11 target)
  IF(${CMAKE_VERSION} VERSION_LESS "3.1")
    IF(NOT MSVC)
      TARGET_COMPILE_OPTIONS(${target} PRIVATE -std=c++11)
    ENDIF()
  ELSE()
    SET_TARGET_PROPERTIES(${target} PROPERTIES
      CXX_STANDARD 11
      CXX_EXTENSIONS NO)
  ENDIF()
ENDMACRO()

MACRO(CPUINFO_TARGET_RUNTIME_LIBRARY target)
  IF(MSVC AND NOT CPUINFO_RUNTIME_TYPE STREQUAL "default")
    IF(CPUINFO_RUNTIME_TYPE STREQUAL "shared")
      TARGET_COMPILE_OPTIONS(${target} PRIVATE
        "/MD$<$<CONFIG:Debug>:d>")
    ELSEIF(CPUINFO_RUNTIME_TYPE STREQUAL "static")
      TARGET_COMPILE_OPTIONS(${target} PRIVATE
        "/MT$<$<CONFIG:Debug>:d>")
    ENDIF()
  ENDIF()
ENDMACRO()

# ---[ Build flags
SET(CPUINFO_SUPPORTED_PLATFORM TRUE)
IF(NOT CMAKE_SYSTEM_PROCESSOR)
  IF(NOT IOS)
    MESSAGE(WARNING
      "Target processor architecture is not specified. "
      "cpuinfo will compile, but cpuinfo_initialize() will always fail.")
    SET(CPUINFO_SUPPORTED_PLATFORM FALSE)
  ENDIF()
ELSEIF(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(i686|AMD64|x86_64|armv5te|armv7|armv7f|armv7s|armv7k|armv7-a|armv7l|arm64|aarch64)$")
  MESSAGE(WARNING
    "Target processor architecture \"${CMAKE_SYSTEM_PROCESSOR}\" is not supported in cpuinfo. "
    "cpuinfo will compile, but cpuinfo_initialize() will always fail.")
  SET(CPUINFO_SUPPORTED_PLATFORM FALSE)
ENDIF()

IF(NOT CMAKE_SYSTEM_NAME)
    MESSAGE(WARNING
      "Target operating system is not specified. "
      "cpuinfo will compile, but cpuinfo_initialize() will always fail.")
  SET(CPUINFO_SUPPORTED_PLATFORM FALSE)
ELSEIF(NOT CMAKE_SYSTEM_NAME MATCHES "^(Windows|Darwin|Linux|Android)$")
  MESSAGE(WARNING
    "Target operating system \"${CMAKE_SYSTEM_NAME}\" is not supported in cpuinfo. "
    "cpuinfo will compile, but cpuinfo_initialize() will always fail.")
  SET(CPUINFO_SUPPORTED_PLATFORM FALSE)
ENDIF()

# ---[ Download deps
SET(CONFU_DEPENDENCIES_SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps
  CACHE PATH "Confu-style dependencies source directory")
SET(CONFU_DEPENDENCIES_BINARY_DIR ${CMAKE_BINARY_DIR}/deps
  CACHE PATH "Confu-style dependencies binary directory")

IF(CPUINFO_BUILD_MOCK_TESTS OR CPUINFO_BUILD_UNIT_TESTS)
  IF(CPUINFO_SUPPORTED_PLATFORM AND NOT DEFINED GOOGLETEST_SOURCE_DIR)
    MESSAGE(STATUS "Downloading Google Test to ${CONFU_DEPENDENCIES_SOURCE_DIR}/googletest (define GOOGLETEST_SOURCE_DIR to avoid it)")
    CONFIGURE_FILE(cmake/DownloadGoogleTest.cmake "${CONFU_DEPENDENCIES_BINARY_DIR}/googletest-download/CMakeLists.txt")
    EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
      WORKING_DIRECTORY "${CONFU_DEPENDENCIES_BINARY_DIR}/googletest-download")
    EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" --build .
      WORKING_DIRECTORY "${CONFU_DEPENDENCIES_BINARY_DIR}/googletest-download")
    SET(GOOGLETEST_SOURCE_DIR "${CONFU_DEPENDENCIES_SOURCE_DIR}/googletest" CACHE STRING "Google Test source directory")
  ENDIF()
ENDIF()

IF(CPUINFO_BUILD_BENCHMARKS)
  IF(CPUINFO_SUPPORTED_PLATFORM AND NOT DEFINED GOOGLEBENCHMARK_SOURCE_DIR)
    MESSAGE(STATUS "Downloading Google Benchmark to ${CONFU_DEPENDENCIES_SOURCE_DIR}/googlebenchmark (define GOOGLEBENCHMARK_SOURCE_DIR to avoid it)")
    CONFIGURE_FILE(cmake/DownloadGoogleBenchmark.cmake "${CONFU_DEPENDENCIES_BINARY_DIR}/googlebenchmark-download/CMakeLists.txt")
    EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
      WORKING_DIRECTORY "${CONFU_DEPENDENCIES_BINARY_DIR}/googlebenchmark-download")
    EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" --build .
      WORKING_DIRECTORY "${CONFU_DEPENDENCIES_BINARY_DIR}/googlebenchmark-download")
    SET(GOOGLEBENCHMARK_SOURCE_DIR "${CONFU_DEPENDENCIES_SOURCE_DIR}/googlebenchmark" CACHE STRING "Google Benchmark source directory")
  ENDIF()
ENDIF()

# ---[ cpuinfo library
SET(CPUINFO_SRCS
  src/init.c
  src/api.c
  src/log.c)

IF(CPUINFO_SUPPORTED_PLATFORM)
  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i686|x86_64|AMD64)$")
    LIST(APPEND CPUINFO_SRCS
      src/x86/init.c
      src/x86/info.c
#      src/x86/vendor.c
#      src/x86/uarch.c
#      src/x86/name.c
      src/x86/topology.c)
#      src/x86/isa.c)
#      src/x86/cache/init.c
#      src/x86/cache/descriptor.c
#      src/x86/cache/deterministic.c)
    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
      LIST(APPEND CPUINFO_SRCS
        src/x86/linux/init.c
        src/x86/linux/cpuinfo.c)
    ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      LIST(APPEND CPUINFO_SRCS src/x86/mach/init.c)
    ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
      LIST(APPEND CPUINFO_SRCS src/x86/windows/init.c)
    ENDIF()
  ENDIF()

  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    LIST(APPEND CPUINFO_SRCS
            src/linux/smallfile.c
            src/linux/multiline.c
            src/linux/cpulist.c
            src/linux/processors.c)
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    LIST(APPEND CPUINFO_SRCS src/mach/topology.c)
  ENDIF()

  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    SET(THREADS_PREFER_PTHREAD_FLAG TRUE)
    FIND_PACKAGE(Threads REQUIRED)
  ENDIF()
ENDIF()

IF(CPUINFO_LIBRARY_TYPE STREQUAL "default")
  ADD_LIBRARY(cpuinfo ${CPUINFO_SRCS})
ELSEIF(CPUINFO_LIBRARY_TYPE STREQUAL "shared")
  ADD_LIBRARY(cpuinfo SHARED ${CPUINFO_SRCS})
ELSEIF(CPUINFO_LIBRARY_TYPE STREQUAL "static")
  ADD_LIBRARY(cpuinfo STATIC ${CPUINFO_SRCS})
ELSE()
  MESSAGE(FATAL_ERROR "Unsupported library type ${CPUINFO_LIBRARY_TYPE}")
ENDIF()
CPUINFO_TARGET_ENABLE_C99(cpuinfo)
CPUINFO_TARGET_RUNTIME_LIBRARY(cpuinfo)
SET_TARGET_PROPERTIES(cpuinfo PROPERTIES PUBLIC_HEADER include/cpuinfo.h)
TARGET_INCLUDE_DIRECTORIES(cpuinfo BEFORE PUBLIC include)
TARGET_INCLUDE_DIRECTORIES(cpuinfo BEFORE PRIVATE src)
IF(CPUINFO_LOG_TO_STDIO)
  TARGET_COMPILE_DEFINITIONS(cpuinfo PRIVATE CPUINFO_LOG_TO_STDIO=1)
ELSE()
  TARGET_COMPILE_DEFINITIONS(cpuinfo PRIVATE CPUINFO_LOG_TO_STDIO=0)
ENDIF()
IF(CPUINFO_SUPPORTED_PLATFORM)
  TARGET_COMPILE_DEFINITIONS(cpuinfo INTERFACE CPUINFO_SUPPORTED_PLATFORM=1)
  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    TARGET_LINK_LIBRARIES(cpuinfo PUBLIC ${CMAKE_THREAD_LIBS_INIT})
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    TARGET_COMPILE_DEFINITIONS(cpuinfo PRIVATE _GNU_SOURCE=1)
  ENDIF()
ELSE()
  TARGET_COMPILE_DEFINITIONS(cpuinfo INTERFACE CPUINFO_SUPPORTED_PLATFORM=0)
ENDIF()

INSTALL(TARGETS cpuinfo
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ---[ cpuinfo micro-benchmarks
IF(CPUINFO_SUPPORTED_PLATFORM AND CPUINFO_BUILD_BENCHMARKS)
  # ---[ Build google benchmark
  IF(NOT TARGET benchmark)
    SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
    ADD_SUBDIRECTORY(
      "${GOOGLEBENCHMARK_SOURCE_DIR}"
      "${CONFU_DEPENDENCIES_BINARY_DIR}/googlebenchmark")
  ENDIF()

  IF(CMAKE_SYSTEM_NAME MATCHES "^(Linux|Android)$")
#    ADD_EXECUTABLE(get-current-bench bench/get-current.cc)
#    TARGET_LINK_LIBRARIES(get-current-bench cpuinfo benchmark)
  ENDIF()

#  ADD_EXECUTABLE(init-bench bench/init.cc)
#  TARGET_LINK_LIBRARIES(init-bench cpuinfo benchmark)
ENDIF()

IF(CPUINFO_SUPPORTED_PLATFORM)
  IF(CPUINFO_BUILD_MOCK_TESTS OR CPUINFO_BUILD_UNIT_TESTS)
    # ---[ Build google test
    IF(NOT TARGET gtest)
      IF(MSVC AND NOT CPUINFO_RUNTIME_TYPE STREQUAL "static")
        SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
      ENDIF()
      ADD_SUBDIRECTORY(
        "${GOOGLETEST_SOURCE_DIR}"
        "${CONFU_DEPENDENCIES_BINARY_DIR}/googletest")
    ENDIF()
  ENDIF()
ENDIF()

# ---[ cpuinfo mock library and mock tests
IF(CPUINFO_SUPPORTED_PLATFORM AND CPUINFO_BUILD_MOCK_TESTS)
  SET(CPUINFO_MOCK_SRCS "${CPUINFO_SRCS}" main.cpp)

  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i686|x86_64)$")
    LIST(APPEND CPUINFO_MOCK_SRCS src/x86/mockcpuid.c)
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    LIST(APPEND CPUINFO_MOCK_SRCS src/linux/mockfile.c)
  ENDIF()

  ADD_LIBRARY(cpuinfo_mock STATIC ${CPUINFO_MOCK_SRCS})
  CPUINFO_TARGET_ENABLE_C99(cpuinfo_mock)
  CPUINFO_TARGET_RUNTIME_LIBRARY(cpuinfo_mock)
  SET_TARGET_PROPERTIES(cpuinfo_mock PROPERTIES PUBLIC_HEADER include/cpuinfo.h)
  TARGET_INCLUDE_DIRECTORIES(cpuinfo_mock BEFORE PUBLIC include)
  TARGET_INCLUDE_DIRECTORIES(cpuinfo_mock BEFORE PRIVATE src)
  TARGET_COMPILE_DEFINITIONS(cpuinfo_mock PUBLIC CPUINFO_MOCK=1)
  TARGET_COMPILE_DEFINITIONS(cpuinfo_mock PRIVATE CPUINFO_LOG_TO_STDIO=1)
  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    TARGET_LINK_LIBRARIES(cpuinfo_mock PUBLIC ${CMAKE_THREAD_LIBS_INIT})
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    TARGET_COMPILE_DEFINITIONS(cpuinfo_mock PRIVATE _GNU_SOURCE=1)
  ENDIF()

ENDIF()

# ---[ cpuinfo unit tests
IF(CPUINFO_SUPPORTED_PLATFORM AND CPUINFO_BUILD_UNIT_TESTS)
  ADD_EXECUTABLE(init-test test/init.cc)
  CPUINFO_TARGET_ENABLE_CXX11(init-test)
  CPUINFO_TARGET_RUNTIME_LIBRARY(init-test)
  TARGET_LINK_LIBRARIES(init-test PRIVATE cpuinfo gtest)
  ADD_TEST(init-test init-test)

  IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    ADD_EXECUTABLE(get-current-test test/get-current.cc)
    CPUINFO_TARGET_ENABLE_CXX11(get-current-test)
    CPUINFO_TARGET_RUNTIME_LIBRARY(get-current-test)
    TARGET_LINK_LIBRARIES(get-current-test PRIVATE cpuinfo gtest)
    ADD_TEST(get-current-test get-current-test)
  ENDIF()

  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i686|x86_64)$")
    ADD_EXECUTABLE(brand-string-test test/name/brand-string.cc)
    CPUINFO_TARGET_ENABLE_CXX11(brand-string-test)
    CPUINFO_TARGET_RUNTIME_LIBRARY(brand-string-test)
    TARGET_LINK_LIBRARIES(brand-string-test PRIVATE cpuinfo gtest gtest_main)
    ADD_TEST(brand-string-test brand-string-test)
  ENDIF()

ENDIF()

# ---[ Helper and debug tools
#IF(CPUINFO_SUPPORTED_PLATFORM AND CPUINFO_BUILD_TOOLS)
#  ADD_EXECUTABLE(isa-info tools/isa-info.c)
#  CPUINFO_TARGET_ENABLE_C99(isa-info)
#  CPUINFO_TARGET_RUNTIME_LIBRARY(isa-info)
#  TARGET_LINK_LIBRARIES(isa-info PRIVATE cpuinfo)
#
#ADD_EXECUTABLE(cpu-info tools/cpu-info.c)
#CPUINFO_TARGET_ENABLE_C99(cpu-info)
#CPUINFO_TARGET_RUNTIME_LIBRARY(cpu-info)
#TARGET_LINK_LIBRARIES(cpu-info PRIVATE cpuinfo)
#
#  ADD_EXECUTABLE(cache-info tools/cache-info.c)
#  CPUINFO_TARGET_ENABLE_C99(cache-info)
#  CPUINFO_TARGET_RUNTIME_LIBRARY(cache-info)
#  TARGET_LINK_LIBRARIES(cache-info PRIVATE cpuinfo)
#
#  IF(CMAKE_SYSTEM_NAME MATCHES "^(Android|Linux)$" AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv5te|armv7|armv7-a|armv7l|arm64|aarch64)$")
#    ADD_EXECUTABLE(auxv-dump tools/auxv-dump.c)
#    CPUINFO_TARGET_ENABLE_C99(auxv-dump)
#    CPUINFO_TARGET_RUNTIME_LIBRARY(auxv-dump)
#    TARGET_LINK_LIBRARIES(auxv-dump PRIVATE ${CMAKE_DL_LIBS})
#
#    ADD_EXECUTABLE(cpuinfo-dump tools/cpuinfo-dump.c)
#    CPUINFO_TARGET_ENABLE_C99(cpuinfo-dump)
#    CPUINFO_TARGET_RUNTIME_LIBRARY(cpuinfo-dump)
#  ENDIF()
#
#  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i686|AMD64|x86_64)$")
#    ADD_EXECUTABLE(cpuid-dump tools/cpuid-dump.c)
#    CPUINFO_TARGET_ENABLE_C99(cpuid-dump)
#    CPUINFO_TARGET_RUNTIME_LIBRARY(cpuid-dump)
#    TARGET_INCLUDE_DIRECTORIES(cpuid-dump BEFORE PRIVATE src)
#    TARGET_INCLUDE_DIRECTORIES(cpuid-dump BEFORE PRIVATE include)
#  ENDIF()
#ENDIF()


add_executable(main main.cpp)
target_link_libraries(main cpuinfo)