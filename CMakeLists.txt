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

FILE(GLOB SOURCE "src/*.h" "src/*.cc" "include/*.h")

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    SET(THREADS_PREFER_PTHREAD_FLAG TRUE)
    FIND_PACKAGE(Threads REQUIRED)
    FILE(GLOB LINUX_SOURCE "src/linux/*.hh" "src/linux/*.cc")
    LIST(APPEND SOURCE ${LINUX_SOURCE})
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(WARNING "${SOURCE}")
    FILE(GLOB DARWIN_SOURCE "src/mach/*.h" "src/mach/*.cc")
    LIST(APPEND SOURCE ${DARWIN_SOURCE})
ENDIF()
message(WARNING "${SOURCE}")

add_executable(main main.cpp ${SOURCE})
target_include_directories(main PUBLIC include)
target_link_libraries(main PUBLIC ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(main cpuinfo)