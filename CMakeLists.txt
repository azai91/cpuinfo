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

set(CMAKE_SYSTEM_NAME Linux)

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

# ---[ cpuinfo library
SET(CPUINFO_SRCS
  src/init.c
  src/api.c
  src/log.c)

IF(CPUINFO_SUPPORTED_PLATFORM)
  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i686|x86_64|AMD64)$")
    LIST(APPEND CPUINFO_SRCS
      src/x86/init.c
      src/x86/topology.c)
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
            src/x86/linux/smallfile.c
            src/x86/linux/multiline.c
            src/x86/linux/cpulist.c
            src/x86/linux/processors.c)
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    LIST(APPEND CPUINFO_SRCS src/x86/mach/topology.c)
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


FILE(GLOB_RECURSE SOURCE "src/*.h" "src/*.c")
message(WARNING "${SOURCE}")
add_executable(main main.cpp ${SOURCE})
target_include_directories(main PUBLIC include)
#target_link_libraries(main cpuinfo)