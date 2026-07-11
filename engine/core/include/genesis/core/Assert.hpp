#pragma once
// =============================================================================
// Genesis::Core::Assert
//
// Asserts de engine: ativos em Debug e em builds "Development"; removidos em
// Release via GENESIS_ENABLE_ASSERTS=0. Nunca lançam exceção — chamam
// std::abort() após logar contexto, porque asserts indicam bug de engine,
// não erro recuperável de runtime (esse é o papel de Result<T,Error>).
// =============================================================================

#include <cstdlib>

#include "genesis/core/Log.hpp"

#ifndef GENESIS_ENABLE_ASSERTS
    #define GENESIS_ENABLE_ASSERTS 1
#endif

namespace genesis::core::detail {

inline void AssertFailed(const char* expr, const char* file, int line, const char* msg) {
    char buffer[512];
    std::snprintf(buffer, sizeof(buffer), "Assertion failed: (%s) at %s:%d — %s",
                  expr, file, line, msg ? msg : "");
    GENESIS_LOG_FATAL("Assert", buffer);
    std::abort();
}

}  // namespace genesis::core::detail

#if GENESIS_ENABLE_ASSERTS
    #define GENESIS_ASSERT(expr) \
        do { if (!(expr)) { ::genesis::core::detail::AssertFailed(#expr, __FILE__, __LINE__, ""); } } while (0)
    #define GENESIS_ASSERT_MSG(expr, msg) \
        do { if (!(expr)) { ::genesis::core::detail::AssertFailed(#expr, __FILE__, __LINE__, msg); } } while (0)
#else
    #define GENESIS_ASSERT(expr) do {} while (0)
    #define GENESIS_ASSERT_MSG(expr, msg) do {} while (0)
#endif
