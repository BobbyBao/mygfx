#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace math {

    using namespace glm;

	inline static const float MATH_PI = pi<float>();
	inline static const float MATH_HALF_PI = pi<float>() / 2;

    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;
    using Vector2 = glm::vec2;
    using Vector3 = glm::vec3;
    using Vector4 = glm::vec4;
    using Quaternion = glm::quat;

	struct Aabb {
		float3 min = float3{std::numeric_limits<float>::max()};
		float3 max = float3{std::numeric_limits<float>::min()};

		static Aabb Empty;
		static Aabb Infinity;
		
		Aabb& merge(const Aabb& other)
		{
			min = glm::min(min, other.min);
			max = glm::max(max, other.max);
			return *this;
		}

		Aabb& merge(const vec3& pt)
		{
			min = glm::min(min, pt);
			max = glm::max(max, pt);
			return *this;
		}

	};

    struct Rect {
        vec2 min;
        vec2 max;
    };

   
    template<typename T>
    inline constexpr T clamp(T v, T min, T max) noexcept {
        assert(min <= max);
        return T(math::min(max, math::max(min, v)));
    }

	
	const mat4 perspective(float fovyRadians, float aspect, float zNear, float zFar, bool invertedDepth);
	const mat4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar, bool invertedDepth);

}

namespace mygfx {
    using namespace math;
}