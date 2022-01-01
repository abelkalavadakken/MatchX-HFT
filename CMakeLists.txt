cmake_minimum_required(VERSION 3.20)
project(NanoTrader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CompilerFlags)
include(Dependencies)

option(ENABLE_BENCHMARKS "Enable benchmarks" ON)
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_PROFILING "Enable profiling" OFF)

include_directories(include)

add_subdirectory(src)

if(ENABLE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(tools)