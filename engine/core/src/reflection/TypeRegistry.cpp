#include "genesis/core/reflection/TypeRegistry.hpp"

#include "genesis/core/Log.hpp"

namespace genesis::core::reflection {

TypeRegistry& TypeRegistry::Instance() {
    static TypeRegistry instance;
    return instance;
}

void TypeRegistry::Register(TypeInfo info) {
    std::scoped_lock lock(m_mutex);
    const std::string name = info.name;
    if (m_types.contains(name)) {
        GENESIS_LOG_WARN("Reflection", ("tipo já registrado, sobrescrevendo: " + name).c_str());
    }
    m_types[name] = std::move(info);
}

const TypeInfo* TypeRegistry::Find(std::string_view name) const {
    std::scoped_lock lock(m_mutex);
    auto it = m_types.find(std::string(name));
    return it != m_types.end() ? &it->second : nullptr;
}

std::vector<const TypeInfo*> TypeRegistry::AllTypes() const {
    std::scoped_lock lock(m_mutex);
    std::vector<const TypeInfo*> result;
    result.reserve(m_types.size());
    for (const auto& [name, info] : m_types) {
        result.push_back(&info);
    }
    return result;
}

std::size_t TypeRegistry::Count() const {
    std::scoped_lock lock(m_mutex);
    return m_types.size();
}

}  // namespace genesis::core::reflection
