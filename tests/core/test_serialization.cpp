#include <catch2/catch_test_macros.hpp>

#include "fixtures/ReflectedTransform.hpp"
#include "genesis/core/serialization/JsonSerializer.hpp"
#include "genesis/core/reflection/TypeRegistry.hpp"
#include "genesis/core/serialization/ReflectionSerializer.hpp"

using namespace genesis::core::serialization;
using namespace genesis::core::reflection;
using genesis::core::math::Vec3;
using genesis::test::ReflectedTransform;

TEST_CASE("Serialize ReflectedTransform to JSON string and deserialize it back", "[serialization]") {
    const TypeInfo* info = TypeRegistry::Instance().Find("genesis::test::ReflectedTransform");
    REQUIRE(info != nullptr);

    ReflectedTransform transform;
    transform.positionX = 10.0f;
    transform.positionY = -5.5f;
    transform.positionZ = 0.1f;
    transform.scale = 2.0f;
    transform.id = 1337;
    transform.active = false;
    transform.velocity = Vec3{1.0f, 2.0f, 3.0f};
    transform.internalCache = 999.0f; // This is NOT reflected, should not be serialized/deserialized

    // 1. Serialize
    JsonSerializer serializer;
    Serialize(serializer, *info, &transform);
    std::string jsonStr = serializer.Dump();

    // Verify it contains our values
    REQUIRE(jsonStr.find("\"positionX\":10.0") != std::string::npos || jsonStr.find("\"positionX\": 10.0") != std::string::npos);
    REQUIRE(jsonStr.find("\"id\":1337") != std::string::npos || jsonStr.find("\"id\": 1337") != std::string::npos);
    REQUIRE(jsonStr.find("\"active\":false") != std::string::npos || jsonStr.find("\"active\": false") != std::string::npos);
    REQUIRE(jsonStr.find("\"velocity\"") != std::string::npos);
    // internalCache must not be in the JSON
    REQUIRE(jsonStr.find("internalCache") == std::string::npos);

    // 2. Deserialize
    ReflectedTransform deserializedTransform;
    deserializedTransform.positionX = 0.0f;
    deserializedTransform.positionY = 0.0f;
    deserializedTransform.positionZ = 0.0f;
    deserializedTransform.scale = 0.0f;
    deserializedTransform.id = 0;
    deserializedTransform.active = true;
    deserializedTransform.velocity = Vec3{0.0f, 0.0f, 0.0f};
    deserializedTransform.internalCache = 123.0f;

    JsonDeserializer deserializer(jsonStr);
    Deserialize(deserializer, *info, &deserializedTransform);

    // Compare fields
    REQUIRE(deserializedTransform.positionX == transform.positionX);
    REQUIRE(deserializedTransform.positionY == transform.positionY);
    REQUIRE(deserializedTransform.positionZ == transform.positionZ);
    REQUIRE(deserializedTransform.scale == transform.scale);
    REQUIRE(deserializedTransform.id == transform.id);
    REQUIRE(deserializedTransform.active == transform.active);
    REQUIRE(deserializedTransform.velocity.x == transform.velocity.x);
    REQUIRE(deserializedTransform.velocity.y == transform.velocity.y);
    REQUIRE(deserializedTransform.velocity.z == transform.velocity.z);
    
    // Check that internalCache was NOT modified by deserialization
    REQUIRE(deserializedTransform.internalCache == 123.0f);
}

TEST_CASE("Serialization Round-trip", "[serialization]") {
    ReflectedTransform transform;
    transform.positionX = 1.0f;
    transform.positionY = 2.0f;
    transform.positionZ = 3.0f;
    transform.scale = 4.0f;
    transform.id = 42;
    transform.active = true;
    transform.velocity = Vec3{7.0f, 8.0f, 9.0f};

    JsonSerializer s;
    bool success = SerializeByName(s, "genesis::test::ReflectedTransform", &transform);
    REQUIRE(success);

    std::string jsonStr = s.Dump();

    ReflectedTransform target;
    JsonDeserializer d(jsonStr);
    bool deserialized = DeserializeByName(d, "genesis::test::ReflectedTransform", &target);
    REQUIRE(deserialized);

    REQUIRE(target.positionX == transform.positionX);
    REQUIRE(target.positionY == transform.positionY);
    REQUIRE(target.positionZ == transform.positionZ);
    REQUIRE(target.scale == transform.scale);
    REQUIRE(target.id == transform.id);
    REQUIRE(target.active == transform.active);
    REQUIRE(target.velocity.x == transform.velocity.x);
    REQUIRE(target.velocity.y == transform.velocity.y);
    REQUIRE(target.velocity.z == transform.velocity.z);
}

TEST_CASE("Unknown type serialization/deserialization returns error", "[serialization]") {
    ReflectedTransform transform;
    JsonSerializer s;
    bool success = SerializeByName(s, "genesis::test::UnknownType", &transform);
    REQUIRE_FALSE(success);

    JsonDeserializer d("{}");
    bool deserialized = DeserializeByName(d, "genesis::test::UnknownType", &transform);
    REQUIRE_FALSE(deserialized);
}
