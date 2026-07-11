#pragma once

#include "genesis/core/serialization/Serializer.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace genesis::core::serialization {

class JsonSerializer final : public ISerializer {
public:
    JsonSerializer();
    ~JsonSerializer() override = default;

    void BeginObject(std::string_view name) override;
    void EndObject() override;

    void WriteFloat(std::string_view key, float value) override;
    void WriteInt(std::string_view key, int value) override;
    void WriteBool(std::string_view key, bool value) override;
    void WriteVec3(std::string_view key, const math::Vec3& value) override;

    [[nodiscard]] std::string Dump(int indent = 4) const;
    [[nodiscard]] const nlohmann::json& GetJson() const { return m_root; }

private:
    nlohmann::json m_root;
    std::vector<nlohmann::json*> m_stack;
};

class JsonDeserializer final : public IDeserializer {
public:
    explicit JsonDeserializer(std::string_view jsonStr);
    explicit JsonDeserializer(const nlohmann::json& jsonVal);
    ~JsonDeserializer() override = default;

    void BeginObject(std::string_view name) override;
    void EndObject() override;

    float ReadFloat(std::string_view key) override;
    int ReadInt(std::string_view key) override;
    bool ReadBool(std::string_view key) override;
    math::Vec3 ReadVec3(std::string_view key) override;

private:
    nlohmann::json m_root;
    std::vector<const nlohmann::json*> m_stack;
};

} // namespace genesis::core::serialization
