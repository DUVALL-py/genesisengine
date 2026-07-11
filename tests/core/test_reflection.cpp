#include <catch2/catch_test_macros.hpp>

#include "fixtures/ReflectedTransform.hpp"
#include "genesis/core/math/Vec.hpp"
#include "genesis/core/reflection/TypeRegistry.hpp"

using namespace genesis::core::reflection;
using genesis::core::math::Vec3;
using genesis::test::ReflectedTransform;

TEST_CASE("TypeRegistry contains the type generated from ReflectedTransform.hpp", "[reflection]") {
    const TypeInfo* info = TypeRegistry::Instance().Find("genesis::test::ReflectedTransform");
    REQUIRE(info != nullptr);
    REQUIRE(info->name == "genesis::test::ReflectedTransform");
    REQUIRE(info->size == sizeof(ReflectedTransform));
}

TEST_CASE("Generated TypeInfo reflects exactly the annotated properties", "[reflection]") {
    const TypeInfo* info = TypeRegistry::Instance().Find("genesis::test::ReflectedTransform");
    REQUIRE(info != nullptr);

    // 7 propriedades anotadas com GENESIS_PROPERTY(); internalCache NÃO deve aparecer.
    REQUIRE(info->properties.size() == 7);
    REQUIRE(info->FindProperty("internalCache") == nullptr);

    REQUIRE(info->FindProperty("positionX") != nullptr);
    REQUIRE(info->FindProperty("positionY") != nullptr);
    REQUIRE(info->FindProperty("positionZ") != nullptr);
    REQUIRE(info->FindProperty("scale") != nullptr);
    REQUIRE(info->FindProperty("id") != nullptr);
    REQUIRE(info->FindProperty("active") != nullptr);
    REQUIRE(info->FindProperty("velocity") != nullptr);
}

TEST_CASE("Offsets discovered by the parser match real memory layout (offsetof do compilador real)", "[reflection]") {
    const TypeInfo* info = TypeRegistry::Instance().Find("genesis::test::ReflectedTransform");
    REQUIRE(info != nullptr);

    const PropertyInfo* posX = info->FindProperty("positionX");
    REQUIRE(posX->offset == offsetof(ReflectedTransform, positionX));

    const PropertyInfo* scale = info->FindProperty("scale");
    REQUIRE(scale->offset == offsetof(ReflectedTransform, scale));

    const PropertyInfo* velocity = info->FindProperty("velocity");
    REQUIRE(velocity->offset == offsetof(ReflectedTransform, velocity));
    REQUIRE(velocity->type == PropertyType::Vec3);
}

TEST_CASE("PropertyInfo::As<T> reads and writes real instance data through the reflected offset", "[reflection]") {
    const TypeInfo* info = TypeRegistry::Instance().Find("genesis::test::ReflectedTransform");
    REQUIRE(info != nullptr);

    ReflectedTransform transform;
    transform.positionX = 1.0f;
    transform.id = 42;
    transform.active = true;
    transform.velocity = Vec3{1.0f, 2.0f, 3.0f};

    const PropertyInfo* posX = info->FindProperty("positionX");
    REQUIRE(posX->As<float>(&transform) == 1.0f);

    // Escreve via reflection e confirma que o campo real da struct mudou.
    posX->As<float>(&transform) = 99.5f;
    REQUIRE(transform.positionX == 99.5f);

    const PropertyInfo* idProp = info->FindProperty("id");
    REQUIRE(idProp->As<int>(&transform) == 42);

    const PropertyInfo* activeProp = info->FindProperty("active");
    REQUIRE(activeProp->As<bool>(&transform) == true);

    const PropertyInfo* velocityProp = info->FindProperty("velocity");
    const Vec3& v = velocityProp->As<Vec3>(&transform);
    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);
}

TEST_CASE("Looking up an unknown type returns nullptr", "[reflection]") {
    REQUIRE(TypeRegistry::Instance().Find("genesis::test::DoesNotExist") == nullptr);
}
