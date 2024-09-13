#include "Maths.h"

namespace math {

Aabb Aabb::Empty;
Aabb Aabb::Infinity{vec3{std::numeric_limits<float>::max()}, vec3 {std::numeric_limits<float>::min()}};

inline const mat4 Matrix4_perspective(float fovyRadians, float aspect, float zNear, float zFar)
{
    static const float VECTORMATH_PI_OVER_2 = 1.570796327f;

    float f, rangeInv;
    f = std::tanf(VECTORMATH_PI_OVER_2 - (0.5f * fovyRadians));
    rangeInv = (1.0f / (zNear - zFar));
    return mat4(vec4((f / aspect), 0.0f, 0.0f, 0.0f),
        vec4(0.0f, f, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, ((zNear + zFar) * rangeInv), -1.0f),
        vec4(0.0f, 0.0f, (((zNear * zFar) * rangeInv) * 2.0f), 0.0f));
}

inline const mat4 Matrix4_frustum(float left, float right, float bottom, float top, float zNear, float zFar)
{
    float sum_rl, sum_tb, sum_nf, inv_rl, inv_tb, inv_nf, n2;
    sum_rl = (right + left);
    sum_tb = (top + bottom);
    sum_nf = (zNear + zFar);
    inv_rl = (1.0f / (right - left));
    inv_tb = (1.0f / (top - bottom));
    inv_nf = (1.0f / (zNear - zFar));
    n2 = (zNear + zNear);
    return mat4(vec4((n2 * inv_rl), 0.0f, 0.0f, 0.0f),
        vec4(0.0f, (n2 * inv_tb), 0.0f, 0.0f),
        vec4((sum_rl * inv_rl), (sum_tb * inv_tb), (sum_nf * inv_nf), -1.0f),
        vec4(0.0f, 0.0f, ((n2 * inv_nf) * zFar), 0.0f));
}

inline const mat4 Matrix4_orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
    float sum_rl, sum_tb, sum_nf, inv_rl, inv_tb, inv_nf;
    sum_rl = (right + left);
    sum_tb = (top + bottom);
    sum_nf = (zNear + zFar);
    inv_rl = (1.0f / (right - left));
    inv_tb = (1.0f / (top - bottom));
    inv_nf = (1.0f / (zNear - zFar));
    return mat4(vec4((inv_rl + inv_rl), 0.0f, 0.0f, 0.0f),
        vec4(0.0f, (inv_tb + inv_tb), 0.0f, 0.0f),
        vec4(0.0f, 0.0f, (inv_nf + inv_nf), 0.0f),
        vec4((-sum_rl * inv_rl), (-sum_tb * inv_tb), (sum_nf * inv_nf), 1.0f));
}

const mat4 perspective(float fovyRadians, float aspect, float zNear, float zFar, bool invertedDepth)
{
    if (invertedDepth) {
        const float cotHalfFovY = cosf(0.5f * fovyRadians) / sinf(0.5f * fovyRadians);
        const float m00 = cotHalfFovY / aspect;
        const float m11 = cotHalfFovY;

        math::vec4 c0(m00, 0.f, 0.f, 0.f);
        math::vec4 c1(0.f, m11, 0.f, 0.f);
        math::vec4 c2(0.f, 0.f, 0.f, -1.f);
        math::vec4 c3(0.f, 0.f, zNear, 0.f);

        return mat4 { c0, c1, c2, c3 };
    } else {
        return Matrix4_perspective(fovyRadians, aspect, zNear, zFar);
    }
}

const mat4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar, bool invertedDepth)
{
    if (invertedDepth)
        return Matrix4_orthographic(left, right, bottom, top, 2.0f * zFar - zNear, zNear);
    else
        return Matrix4_orthographic(left, right, bottom, top, 2.0f * zNear - zFar, zFar);
}

}
