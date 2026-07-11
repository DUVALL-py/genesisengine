#pragma once
// =============================================================================
// Genesis::Core::Math::Mat4
//
// Matriz 4x4, column-major (compatível com Vulkan/OpenGL/GLSL sem transpor).
// Armazenamento: m[col][row]. Multiplicação de matrizes e transformação de
// pontos seguem a convenção column-vector (v' = M * v).
// =============================================================================

#include <cmath>

#include "genesis/core/math/Vec.hpp"

namespace genesis::core::math {

struct Mat4 {
    // m[col][row]
    float m[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };

    static constexpr Mat4 Identity() { return Mat4{}; }

    static constexpr Mat4 Translation(const Vec3& t) {
        Mat4 out = Identity();
        out.m[3][0] = t.x;
        out.m[3][1] = t.y;
        out.m[3][2] = t.z;
        return out;
    }

    static constexpr Mat4 Scale(const Vec3& s) {
        Mat4 out = Identity();
        out.m[0][0] = s.x;
        out.m[1][1] = s.y;
        out.m[2][2] = s.z;
        return out;
    }

    static Mat4 Perspective(float fovYRadians, float aspect, float nearZ, float farZ) {
        Mat4 out{};
        for (auto& col : out.m) {
            for (float& value : col) value = 0.0f;
        }
        const float tanHalfFovy = std::tan(fovYRadians * 0.5f);
        out.m[0][0] = 1.0f / (aspect * tanHalfFovy);
        out.m[1][1] = 1.0f / tanHalfFovy;
        out.m[2][2] = farZ / (nearZ - farZ);
        out.m[2][3] = -1.0f;
        out.m[3][2] = -(farZ * nearZ) / (farZ - nearZ);
        return out;
    }

    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        const Vec3 f = (target - eye).Normalized();
        const Vec3 s = f.Cross(up).Normalized();
        const Vec3 u = s.Cross(f);

        Mat4 out = Identity();
        out.m[0][0] = s.x; out.m[1][0] = s.y; out.m[2][0] = s.z;
        out.m[0][1] = u.x; out.m[1][1] = u.y; out.m[2][1] = u.z;
        out.m[0][2] = -f.x; out.m[1][2] = -f.y; out.m[2][2] = -f.z;
        out.m[3][0] = -s.Dot(eye);
        out.m[3][1] = -u.Dot(eye);
        out.m[3][2] = f.Dot(eye);
        return out;
    }

    [[nodiscard]] Mat4 operator*(const Mat4& rhs) const {
        Mat4 out{};
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += m[k][row] * rhs.m[col][k];
                }
                out.m[col][row] = sum;
            }
        }
        return out;
    }

    [[nodiscard]] Vec4 operator*(const Vec4& v) const {
        Vec4 out{};
        out.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w;
        out.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w;
        out.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w;
        out.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w;
        return out;
    }
};

}  // namespace genesis::core::math
