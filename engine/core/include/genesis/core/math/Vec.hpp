#pragma once
// =============================================================================
// Genesis::Core::Math::Vec3 / Vec4
//
// Tipos de vetor mínimos, constexpr-friendly, sem dependência de SIMD ainda
// (isso vem como otimização posterior, sem mudar a API pública). Layout de
// memória é POD (float x,y,z[,w]) para ser diretamente compatível com
// buffers de GPU.
// =============================================================================

#include <cmath>

namespace genesis::core::math {

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    constexpr Vec3() = default;
    constexpr Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    constexpr Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    constexpr Vec3 operator-() const { return {-x, -y, -z}; }

    constexpr Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    constexpr Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    constexpr Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    [[nodiscard]] constexpr float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }

    [[nodiscard]] constexpr Vec3 Cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }

    [[nodiscard]] float Length() const { return std::sqrt(Dot(*this)); }

    [[nodiscard]] constexpr float LengthSquared() const { return Dot(*this); }

    [[nodiscard]] Vec3 Normalized() const {
        const float len = Length();
        if (len <= 1e-8f) return {0.0f, 0.0f, 0.0f};
        const float inv = 1.0f / len;
        return {x * inv, y * inv, z * inv};
    }

    static constexpr Vec3 Zero() { return {0.0f, 0.0f, 0.0f}; }
    static constexpr Vec3 One() { return {1.0f, 1.0f, 1.0f}; }
    static constexpr Vec3 UnitX() { return {1.0f, 0.0f, 0.0f}; }
    static constexpr Vec3 UnitY() { return {0.0f, 1.0f, 0.0f}; }
    static constexpr Vec3 UnitZ() { return {0.0f, 0.0f, 1.0f}; }
};

struct Vec4 {
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

    constexpr Vec4() = default;
    constexpr Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    constexpr Vec4(const Vec3& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}

    [[nodiscard]] constexpr float Dot(const Vec4& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
};

inline constexpr bool NearlyEqual(float a, float b, float epsilon = 1e-5f) {
    const float diff = a - b;
    return (diff < 0 ? -diff : diff) <= epsilon;
}

}  // namespace genesis::core::math
