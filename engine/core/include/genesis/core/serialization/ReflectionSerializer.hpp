#pragma once

#include "genesis/core/reflection/TypeInfo.hpp"
#include "genesis/core/serialization/Serializer.hpp"
#include <string_view>

namespace genesis::core::serialization {

// Serializes an instance of a reflected type using its TypeInfo metadata
void Serialize(ISerializer& s, const reflection::TypeInfo& info, const void* instance);

// Deserializes an instance of a reflected type using its TypeInfo metadata
void Deserialize(IDeserializer& d, const reflection::TypeInfo& info, void* instance);

// Serializes an instance by looking up typeName in the TypeRegistry. Returns false on unknown type.
bool SerializeByName(ISerializer& s, std::string_view typeName, const void* instance);

// Deserializes an instance by looking up typeName in the TypeRegistry. Returns false on unknown type.
bool DeserializeByName(IDeserializer& d, std::string_view typeName, void* instance);

} // namespace genesis::core::serialization
