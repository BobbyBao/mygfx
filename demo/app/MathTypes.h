#pragma once


namespace math {

	struct float2 {
		float x{ 0 }, y{ 0 };
	};

	struct float3 {
		float x{ 0 }, y{ 0 }, z{ 0 };
	};

	struct float4 {
		float x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 };
	};

	struct Aabb {
		float3 min, max;
	};

    template<typename T>
    inline constexpr T min(T a, T b) noexcept {
        return a < b ? a : b;
    }

    template<typename T>
    inline constexpr T max(T a, T b) noexcept {
        return a > b ? a : b;
    }

    template<typename T>
    inline constexpr T clamp(T v, T min, T max) noexcept {
        assert(min <= max);
        return T(math::min(max, math::max(min, v)));
    }
}

namespace mygfx {
    using namespace math;
}