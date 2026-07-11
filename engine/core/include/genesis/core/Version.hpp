#pragma once

namespace genesis::core {

struct Version {
    int major;
    int minor;
    int patch;
};

// Versão do módulo Core. Incrementada manualmente por enquanto; automatizar
// via CMake/git describe é tarefa futura do Build System.
[[nodiscard]] Version GetCoreVersion() noexcept;
[[nodiscard]] const char* GetCoreVersionString() noexcept;

}  // namespace genesis::core
