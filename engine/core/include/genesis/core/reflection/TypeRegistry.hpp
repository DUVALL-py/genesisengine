#pragma once
// =============================================================================
// Genesis::Core::Reflection::TypeRegistry
//
// Registro global de TypeInfo, populado em tempo de static-init pelo código
// .generated.cpp de cada header anotado com GENESIS_CLASS(). Thread-safe
// porque o Asset Pipeline e o Editor (fases futuras) podem consultar tipos
// de threads diferentes.
// =============================================================================

#include <mutex>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "genesis/core/reflection/TypeInfo.hpp"

namespace genesis::core::reflection {

class TypeRegistry {
public:
    static TypeRegistry& Instance();

    void Register(TypeInfo info);

    [[nodiscard]] const TypeInfo* Find(std::string_view name) const;
    [[nodiscard]] std::vector<const TypeInfo*> AllTypes() const;
    [[nodiscard]] std::size_t Count() const;

private:
    TypeRegistry() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, TypeInfo> m_types;
};

}  // namespace genesis::core::reflection
