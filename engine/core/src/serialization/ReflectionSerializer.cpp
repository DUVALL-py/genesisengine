#include "genesis/core/serialization/ReflectionSerializer.hpp"
#include "genesis/core/reflection/TypeRegistry.hpp"
#include "genesis/core/Log.hpp"

namespace genesis::core::serialization {

void Serialize(ISerializer& s, const reflection::TypeInfo& info, const void* instance) {
    s.BeginObject(info.name);
    for (const auto& prop : info.properties) {
        switch (prop.type) {
            case reflection::PropertyType::Float:
                s.WriteFloat(prop.name, prop.As<float>(instance));
                break;
            case reflection::PropertyType::Int32:
                s.WriteInt(prop.name, prop.As<int>(instance));
                break;
            case reflection::PropertyType::Bool:
                s.WriteBool(prop.name, prop.As<bool>(instance));
                break;
            case reflection::PropertyType::Vec3:
                s.WriteVec3(prop.name, prop.As<math::Vec3>(instance));
                break;
            default:
                GENESIS_LOG_WARN("Serialization", "Propriedade '%s' do tipo '%s' tem tipo desconhecido e será ignorada", 
                                 prop.name.c_str(), info.name.c_str());
                break;
        }
    }
    s.EndObject();
}

void Deserialize(IDeserializer& d, const reflection::TypeInfo& info, void* instance) {
    d.BeginObject(info.name);
    for (const auto& prop : info.properties) {
        switch (prop.type) {
            case reflection::PropertyType::Float:
                prop.As<float>(instance) = d.ReadFloat(prop.name);
                break;
            case reflection::PropertyType::Int32:
                prop.As<int>(instance) = d.ReadInt(prop.name);
                break;
            case reflection::PropertyType::Bool:
                prop.As<bool>(instance) = d.ReadBool(prop.name);
                break;
            case reflection::PropertyType::Vec3:
                prop.As<math::Vec3>(instance) = d.ReadVec3(prop.name);
                break;
            default:
                GENESIS_LOG_WARN("Serialization", "Propriedade '%s' do tipo '%s' tem tipo desconhecido e será ignorada", 
                                 prop.name.c_str(), info.name.c_str());
                break;
        }
    }
    d.EndObject();
}

bool SerializeByName(ISerializer& s, std::string_view typeName, const void* instance) {
    const auto* info = reflection::TypeRegistry::Instance().Find(typeName);
    if (!info) {
        GENESIS_LOG_ERROR("Serialization", "Falha ao serializar: tipo '%s' não está registrado em TypeRegistry", std::string(typeName).c_str());
        return false;
    }
    Serialize(s, *info, instance);
    return true;
}

bool DeserializeByName(IDeserializer& d, std::string_view typeName, void* instance) {
    const auto* info = reflection::TypeRegistry::Instance().Find(typeName);
    if (!info) {
        GENESIS_LOG_ERROR("Serialization", "Falha ao deserializar: tipo '%s' não está registrado em TypeRegistry", std::string(typeName).c_str());
        return false;
    }
    Deserialize(d, *info, instance);
    return true;
}

} // namespace genesis::core::serialization
