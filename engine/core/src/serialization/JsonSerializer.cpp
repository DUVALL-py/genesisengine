#include "genesis/core/serialization/JsonSerializer.hpp"

namespace genesis::core::serialization {

// =============================================================================
// JsonSerializer
// =============================================================================

JsonSerializer::JsonSerializer() {
    m_stack.push_back(&m_root);
}

void JsonSerializer::BeginObject(std::string_view name) {
    nlohmann::json* current = m_stack.back();
    std::string key(name);
    (*current)[key] = nlohmann::json::object();
    m_stack.push_back(&((*current)[key]));
}

void JsonSerializer::EndObject() {
    if (m_stack.size() > 1) {
        m_stack.pop_back();
    }
}

void JsonSerializer::WriteFloat(std::string_view key, float value) {
    (*m_stack.back())[std::string(key)] = value;
}

void JsonSerializer::WriteInt(std::string_view key, int value) {
    (*m_stack.back())[std::string(key)] = value;
}

void JsonSerializer::WriteBool(std::string_view key, bool value) {
    (*m_stack.back())[std::string(key)] = value;
}

void JsonSerializer::WriteVec3(std::string_view key, const math::Vec3& value) {
    nlohmann::json vecObj = nlohmann::json::object();
    vecObj["x"] = value.x;
    vecObj["y"] = value.y;
    vecObj["z"] = value.z;
    (*m_stack.back())[std::string(key)] = vecObj;
}

std::string JsonSerializer::Dump(int indent) const {
    return m_root.dump(indent);
}

// =============================================================================
// JsonDeserializer
// =============================================================================

JsonDeserializer::JsonDeserializer(std::string_view jsonStr) {
    m_root = nlohmann::json::parse(jsonStr);
    m_stack.push_back(&m_root);
}

JsonDeserializer::JsonDeserializer(const nlohmann::json& jsonVal) : m_root(jsonVal) {
    m_stack.push_back(&m_root);
}

void JsonDeserializer::BeginObject(std::string_view name) {
    const nlohmann::json* current = m_stack.back();
    std::string key(name);
    if (current && current->contains(key) && (*current)[key].is_object()) {
        m_stack.push_back(&((*current)[key]));
    } else {
        m_stack.push_back(nullptr);
    }
}

void JsonDeserializer::EndObject() {
    if (m_stack.size() > 1) {
        m_stack.pop_back();
    }
}

float JsonDeserializer::ReadFloat(std::string_view key) {
    const nlohmann::json* current = m_stack.back();
    if (current && current->contains(std::string(key))) {
        const auto& val = (*current)[std::string(key)];
        if (val.is_number()) {
            return val.get<float>();
        }
    }
    return 0.0f;
}

int JsonDeserializer::ReadInt(std::string_view key) {
    const nlohmann::json* current = m_stack.back();
    if (current && current->contains(std::string(key))) {
        const auto& val = (*current)[std::string(key)];
        if (val.is_number()) {
            return val.get<int>();
        }
    }
    return 0;
}

bool JsonDeserializer::ReadBool(std::string_view key) {
    const nlohmann::json* current = m_stack.back();
    if (current && current->contains(std::string(key))) {
        const auto& val = (*current)[std::string(key)];
        if (val.is_boolean()) {
            return val.get<bool>();
        }
    }
    return false;
}

math::Vec3 JsonDeserializer::ReadVec3(std::string_view key) {
    const nlohmann::json* current = m_stack.back();
    if (current && current->contains(std::string(key))) {
        const auto& val = (*current)[std::string(key)];
        if (val.is_object()) {
            float x = val.contains("x") && val["x"].is_number() ? val["x"].get<float>() : 0.0f;
            float y = val.contains("y") && val["y"].is_number() ? val["y"].get<float>() : 0.0f;
            float z = val.contains("z") && val["z"].is_number() ? val["z"].get<float>() : 0.0f;
            return math::Vec3{x, y, z};
        }
    }
    return math::Vec3{0.0f, 0.0f, 0.0f};
}

} // namespace genesis::core::serialization
