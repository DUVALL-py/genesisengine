#pragma once
// =============================================================================
// Genesis::Core::Reflection::TypeInfo
//
// Estruturas de dados de runtime preenchidas pelo código .generated.cpp
// (emitido por tools/reflect/genesis_reflect.py a partir do AST real do
// header). Nada aqui é escrito à mão para um tipo específico — isto é
// infraestrutura genérica consumida pelo código gerado.
// =============================================================================

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "genesis/core/math/Vec.hpp"

namespace genesis::core::reflection {

enum class PropertyType {
    Float,
    Int32,
    Bool,
    Vec3,
    Unknown,
};

[[nodiscard]] inline const char* ToString(PropertyType type) noexcept {
    switch (type) {
        case PropertyType::Float: return "Float";
        case PropertyType::Int32: return "Int32";
        case PropertyType::Bool:  return "Bool";
        case PropertyType::Vec3:  return "Vec3";
        case PropertyType::Unknown: return "Unknown";
    }
    return "Unknown";
}

// Mapeamento tipo C++ -> PropertyType, usado para checar em runtime que
// PropertyInfo::As<T>() está sendo chamado com o T correto.
template <typename T> struct PropertyTypeOf { static constexpr PropertyType value = PropertyType::Unknown; };
template <> struct PropertyTypeOf<float> { static constexpr PropertyType value = PropertyType::Float; };
template <> struct PropertyTypeOf<int>   { static constexpr PropertyType value = PropertyType::Int32; };
template <> struct PropertyTypeOf<bool>  { static constexpr PropertyType value = PropertyType::Bool; };
template <> struct PropertyTypeOf<genesis::core::math::Vec3> { static constexpr PropertyType value = PropertyType::Vec3; };

struct PropertyInfo {
    std::string name;
    PropertyType type = PropertyType::Unknown;
    std::size_t offset = 0;
    std::size_t size = 0;

    // Acesso tipado a uma instância via offset cru. GENESIS_ASSERT falha se T
    // não corresponder ao PropertyType registrado — protege contra o erro
    // clássico de reflection de ler um int como float.
    template <typename T>
    [[nodiscard]] T& As(void* instance) const;

    template <typename T>
    [[nodiscard]] const T& As(const void* instance) const;
};

struct TypeInfo {
    std::string name;
    std::size_t size = 0;
    std::vector<PropertyInfo> properties;

    [[nodiscard]] const PropertyInfo* FindProperty(std::string_view propName) const {
        for (const auto& prop : properties) {
            if (prop.name == propName) return &prop;
        }
        return nullptr;
    }
};

}  // namespace genesis::core::reflection

#include "genesis/core/reflection/TypeInfo.inl"
