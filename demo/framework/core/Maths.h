#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace math {

    using namespace glm;

    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;
    using Vector2 = glm::vec2;
    using Vector3 = glm::vec3;
    using Vector4 = glm::vec4;
    using Quaternion = glm::quat;

	inline static const float MATH_PI = pi<float>();
	inline static const float MATH_HALF_PI = pi<float>() / 2;
	
	inline constexpr vec3 FORWARD(0.0f, 0.0f, -1.0f);
	inline constexpr vec3 BACK(0.0f, 0.0f, 1.0);
	inline constexpr vec3 RIGHT(1.0f, 0.0f, 0.0f);
	inline constexpr vec3 LEFT(-1.0f, 0.0f, 0.0f);
	inline constexpr vec3 UP(0.0f, 1.0f, 0.0f);
	inline constexpr vec3 DOWN(0.0f, -1.0f, 0.0f);
    

	struct Aabb {
		float3 min = float3{std::numeric_limits<float>::max()};
		float3 max = float3{std::numeric_limits<float>::min()};

		static Aabb Empty;
		static Aabb Infinity;

		float3 size() const {
			return max - min;
		}

		float3 center() const {
			return (max + min) / 2.0f;
		}
		
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