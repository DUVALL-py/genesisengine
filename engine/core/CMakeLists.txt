include(FetchContent)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(genesis_core STATIC
    src/Version.cpp
    src/jobs/JobSystem.cpp
    src/reflection/TypeRegistry.cpp
    src/serialization/JsonSerializer.cpp
    src/serialization/ReflectionSerializer.cpp
)

add_library(genesis::core ALIAS genesis_core)

target_include_directories(genesis_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(genesis_core PUBLIC genesis_compiler_flags)

find_package(Threads REQUIRED)
target_link_libraries(genesis_core PUBLIC Threads::Threads nlohmann_json::nlohmann_json)
