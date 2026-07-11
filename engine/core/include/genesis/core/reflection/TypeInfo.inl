#pragma once
// Implementação separada em .inl para manter TypeInfo.hpp legível; incluído
// no fim de TypeInfo.hpp. Não incluir diretamente.

#include <cstddef>

#include "genesis/core/Assert.hpp"

namespace genesis::core::reflection {

template <typename T>
T& PropertyInfo::As(void* instance) const {
    GENESIS_ASSERT_MSG(PropertyTypeOf<T>::value == type, "PropertyInfo::As<T>: tipo T não corresponde ao PropertyType registrado");
    return *reinterpret_cast<T*>(static_cast<std::byte*>(instance) + offset);
}

template <typename T>
const T& PropertyInfo::As(const void* instance) const {
    GENESIS_ASSERT_MSG(PropertyTypeOf<T>::value == type, "PropertyInfo::As<T>: tipo T não corresponde ao PropertyType registrado");
    return *reinterpret_cast<const T*>(static_cast<const std::byte*>(instance) + offset);
}

}  // namespace genesis::core::reflection
