#pragma once

#include <string_view>
#include "genesis/core/math/Vec.hpp"

namespace genesis::core::serialization {

class ISerializer {
public:
    virtual ~ISerializer() = default;

    virtual void BeginObject(std::string_view name) = 0;
    virtual void EndObject() = 0;

    virtual void WriteFloat(std::string_view key, float value) = 0;
    virtual void WriteInt(std::string_view key, int value) = 0;
    virtual void WriteBool(std::string_view key, bool value) = 0;
    virtual void WriteVec3(std::string_view key, const math::Vec3& value) = 0;
};

class IDeserializer {
public:
    virtual ~IDeserializer() = default;

    virtual void BeginObject(std::string_view name) = 0;
    virtual void EndObject() = 0;

    virtual float ReadFloat(std::string_view key) = 0;
    virtual int ReadInt(std::string_view key) = 0;
    virtual bool ReadBool(std::string_view key) = 0;
    virtual math::Vec3 ReadVec3(std::string_view key) = 0;
};

} // namespace genesis::core::serialization
