#pragma once

#include "Volk/volk.h"

#include <array>

#include "vk_mem_alloc.h"

namespace mygfx {


	static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 16;
	static constexpr uint32_t VARIABLE_DESC_COUNT = 4096;

	struct DescriptorResourceCounts : public std::array<uint32_t, DESCRIPTOR_TYPE_COUNT> {

		bool operator==(const DescriptorResourceCounts& other) {
			return std::memcmp(data(), other.data(), sizeof(uint32_t)*DESCRIPTOR_TYPE_COUNT) == 0;
		}
	};
}