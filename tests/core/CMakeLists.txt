include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.6.0
)
FetchContent_MakeAvailable(catch2)

genesis_add_reflected_header(
    HEADER ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/ReflectedTransform.hpp
    INCLUDE_AS "fixtures/ReflectedTransform.hpp"
    OUT_SOURCE_VAR REFLECTED_TRANSFORM_GENERATED_CPP
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/engine/core/include
)

add_executable(genesis_core_tests
    test_memory.cpp
    test_math.cpp
    test_jobs.cpp
    test_reflection.cpp
    test_serialization.cpp
    ${REFLECTED_TRANSFORM_GENERATED_CPP}
)

target_include_directories(genesis_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(genesis_core_tests PRIVATE
    genesis::core
    Catch2::Catch2WithMain
)

include(CTest)
include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(genesis_core_tests)
