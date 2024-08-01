#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace math {

    using namespace glm;

    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;

	struct Aabb {
		float3 min, max;
	};

    template<typename T>
    inline constexpr T clamp(T v, T min, T max) noexcept {
        assert(min <= max);
        return T(math::min(max, math::max(min, v)));
    }

    struct Rect {
        vec2 min;
        vec2 max;
    };
}

namespace mygfx {
    using namespace math;
}