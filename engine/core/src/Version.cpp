#include "genesis/core/Version.hpp"

namespace genesis::core {

Version GetCoreVersion() noexcept { return Version{0, 1, 0}; }

const char* GetCoreVersionString() noexcept { return "0.1.0-mvp"; }

}  // namespace genesis::core
