#pragma once
// =============================================================================
// Genesis::Core::Platform
//
// Camada de detecção de plataforma. Nenhum outro módulo deve fazer #ifdef de
// sistema operacional diretamente — tudo passa por este header, para que
// trocar/adicionar uma plataforma seja uma mudança localizada.
// =============================================================================

#if defined(_WIN32) || defined(_WIN64)
    #define GENESIS_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #define GENESIS_PLATFORM_MACOS 1
#elif defined(__linux__)
    #define GENESIS_PLATFORM_LINUX 1
#else
    #error "Genesis Engine: unsupported platform"
#endif

#if !defined(GENESIS_PLATFORM_WINDOWS)
    #define GENESIS_PLATFORM_WINDOWS 0
#endif
#if !defined(GENESIS_PLATFORM_MACOS)
    #define GENESIS_PLATFORM_MACOS 0
#endif
#if !defined(GENESIS_PLATFORM_LINUX)
    #define GENESIS_PLATFORM_LINUX 0
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #define GENESIS_ARCH_X86_64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define GENESIS_ARCH_ARM64 1
#endif

#if defined(__clang__)
    #define GENESIS_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define GENESIS_COMPILER_GCC 1
#elif defined(_MSC_VER)
    #define GENESIS_COMPILER_MSVC 1
#endif

namespace genesis::core {

enum class OperatingSystem {
    Windows,
    MacOS,
    Linux,
};

[[nodiscard]] constexpr OperatingSystem CurrentOS() noexcept {
#if GENESIS_PLATFORM_WINDOWS
    return OperatingSystem::Windows;
#elif GENESIS_PLATFORM_MACOS
    return OperatingSystem::MacOS;
#else
    return OperatingSystem::Linux;
#endif
}

[[nodiscard]] constexpr const char* CurrentOSName() noexcept {
    switch (CurrentOS()) {
        case OperatingSystem::Windows: return "Windows";
        case OperatingSystem::MacOS:   return "macOS";
        case OperatingSystem::Linux:   return "Linux";
    }
    return "Unknown";
}

}  // namespace genesis::core
