#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "genesis/core/math/Mat4.hpp"
#include "genesis/core/math/Vec.hpp"

using namespace genesis::core::math;
using Catch::Approx;

TEST_CASE("Vec3 basic arithmetic", "[math][vec3]") {
    const Vec3 a{1.0f, 2.0f, 3.0f};
    const Vec3 b{4.0f, 5.0f, 6.0f};

    const Vec3 sum = a + b;
    REQUIRE(sum.x == Approx(5.0f));
    REQUIRE(sum.y == Approx(7.0f));
    REQUIRE(sum.z == Approx(9.0f));

    REQUIRE(a.Dot(b) == Approx(32.0f));  // 1*4 + 2*5 + 3*6

    const Vec3 cross = Vec3::UnitX().Cross(Vec3::UnitY());
    REQUIRE(cross.x == Approx(0.0f));
    REQUIRE(cross.y == Approx(0.0f));
    REQUIRE(cross.z == Approx(1.0f));
}

TEST_CASE("Vec3 normalization", "[math][vec3]") {
    const Vec3 v{3.0f, 0.0f, 4.0f};
    const Vec3 n = v.Normalized();
    REQUIRE(n.Length() == Approx(1.0f).margin(1e-5f));
    REQUIRE(n.x == Approx(0.6f));
    REQUIRE(n.y == Approx(0.0f));
    REQUIRE(n.z == Approx(0.8f));
}

TEST_CASE("Vec3 normalizing a zero vector does not divide by zero", "[math][vec3]") {
    const Vec3 v{0.0f, 0.0f, 0.0f};
    const Vec3 n = v.Normalized();
    REQUIRE(n.x == Approx(0.0f));
    REQUIRE(n.y == Approx(0.0f));
    REQUIRE(n.z == Approx(0.0f));
}

TEST_CASE("Mat4 identity leaves vector unchanged", "[math][mat4]") {
    const Mat4 id = Mat4::Identity();
    const Vec4 v{1.0f, 2.0f, 3.0f, 1.0f};
    const Vec4 result = id * v;

    REQUIRE(result.x == Approx(v.x));
    REQUIRE(result.y == Approx(v.y));
    REQUIRE(result.z == Approx(v.z));
    REQUIRE(result.w == Approx(v.w));
}

TEST_CASE("Mat4 translation moves a point", "[math][mat4]") {
    const Mat4 t = Mat4::Translation(Vec3{10.0f, 0.0f, 0.0f});
    const Vec4 p{0.0f, 0.0f, 0.0f, 1.0f};
    const Vec4 result = t * p;

    REQUIRE(result.x == Approx(10.0f));
    REQUIRE(result.y == Approx(0.0f));
    REQUIRE(result.z == Approx(0.0f));
}

TEST_CASE("Mat4 multiplication composes transforms (translate * scale)", "[math][mat4]") {
    const Mat4 translate = Mat4::Translation(Vec3{5.0f, 0.0f, 0.0f});
    const Mat4 scale = Mat4::Scale(Vec3{2.0f, 2.0f, 2.0f});
    const Mat4 combined = translate * scale;

    const Vec4 p{1.0f, 1.0f, 1.0f, 1.0f};
    const Vec4 result = combined * p;

    // Escala primeiro (1,1,1 -> 2,2,2), depois translada (+5 em x)
    REQUIRE(result.x == Approx(7.0f));
    REQUIRE(result.y == Approx(2.0f));
    REQUIRE(result.z == Approx(2.0f));
}
