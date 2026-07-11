#pragma once
// =============================================================================
// Genesis::Core::Reflection — macros de anotação
//
// GENESIS_CLASS() e GENESIS_PROPERTY() marcam declarações para serem
// descobertas por tools/reflect/genesis_reflect.py, que faz parsing real do
// header via libclang (AST de verdade, não regex e não registro manual).
//
// Truque importante: as macros só expandem para
// __attribute__((annotate(...))) quando GENESIS_REFLECT_TOOL está definido —
// o que só acontece quando a própria ferramenta invoca o compilador (via
// libclang) para parsear o header. Numa compilação normal da engine (GCC/
// Clang de produção, sem essa macro) elas expandem para nada: zero custo de
// build, zero warning, zero binário extra. Isso evita depender de um
// atributo que o GCC de produção não aceita nessas posições (testado: GCC
// rejeita 'annotate' em struct/campo sob -Werror=attributes).
// =============================================================================

#if defined(GENESIS_REFLECT_TOOL)
    #define GENESIS_CLASS() __attribute__((annotate("genesis::reflect::class")))
    #define GENESIS_PROPERTY() __attribute__((annotate("genesis::reflect::property")))
#else
    #define GENESIS_CLASS()
    #define GENESIS_PROPERTY()
#endif
