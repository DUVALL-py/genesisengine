cmake_minimum_required(VERSION 3.24)
project(GenesisEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GENESIS_BUILD_TESTS "Build Genesis unit tests" ON)
option(GENESIS_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# ---------------------------------------------------------------------------
# Common compile options applied to every Genesis target via this interface
# library, instead of being duplicated per-module CMakeLists.txt.
# ---------------------------------------------------------------------------
add_library(genesis_compiler_flags INTERFACE)
target_compile_features(genesis_compiler_flags INTERFACE cxx_std_20)
target_compile_options(genesis_compiler_flags INTERFACE
    -Wall -Wextra -Wpedantic
    $<$<BOOL:${GENESIS_WARNINGS_AS_ERRORS}>:-Werror>
)

enable_testing()

include(cmake/GenesisReflection.cmake)

add_subdirectory(engine/core)

if(GENESIS_BUILD_TESTS)
    add_subdirectory(tests/core)
endif()
