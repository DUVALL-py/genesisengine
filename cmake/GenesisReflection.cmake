# =============================================================================
# GenesisReflection.cmake
#
# Fornece genesis_add_reflected_header(), que registra um custom command
# invocando tools/reflect/genesis_reflect.py (parser real via libclang) sobre
# um header anotado com GENESIS_CLASS()/GENESIS_PROPERTY(), produzindo um
# <Nome>.generated.cpp que deve ser adicionado às fontes do alvo que consome
# aquele tipo refletido.
# =============================================================================

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(GENESIS_REFLECT_TOOL_SCRIPT "${CMAKE_SOURCE_DIR}/tools/reflect/genesis_reflect.py")

# genesis_add_reflected_header(
#     HEADER <caminho absoluto do header>
#     INCLUDE_AS <caminho usado no #include gerado, ex: "genesis/core/foo/Bar.hpp">
#     OUT_SOURCE_VAR <variável que receberá o caminho do .generated.cpp>
#     INCLUDE_DIRS <diretórios extra passados como -I ao parser>
# )
function(genesis_add_reflected_header)
    set(oneValueArgs HEADER INCLUDE_AS OUT_SOURCE_VAR)
    set(multiValueArgs INCLUDE_DIRS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_HEADER OR NOT ARG_INCLUDE_AS OR NOT ARG_OUT_SOURCE_VAR)
        message(FATAL_ERROR "genesis_add_reflected_header: HEADER, INCLUDE_AS e OUT_SOURCE_VAR são obrigatórios")
    endif()

    get_filename_component(header_name ${ARG_HEADER} NAME_WE)
    set(generated_cpp "${CMAKE_CURRENT_BINARY_DIR}/generated/${header_name}.generated.cpp")

    set(include_dir_args "")
    foreach(dir ${ARG_INCLUDE_DIRS})
        list(APPEND include_dir_args "-I${dir}")
    endforeach()

    add_custom_command(
        OUTPUT ${generated_cpp}
        COMMAND ${Python3_EXECUTABLE} ${GENESIS_REFLECT_TOOL_SCRIPT}
                ${ARG_HEADER}
                --include-as ${ARG_INCLUDE_AS}
                -o ${generated_cpp}
                ${include_dir_args}
        DEPENDS ${ARG_HEADER} ${GENESIS_REFLECT_TOOL_SCRIPT}
        COMMENT "Genesis Reflection: parseando ${header_name} via libclang"
        VERBATIM
    )

    set(${ARG_OUT_SOURCE_VAR} ${generated_cpp} PARENT_SCOPE)
endfunction()
