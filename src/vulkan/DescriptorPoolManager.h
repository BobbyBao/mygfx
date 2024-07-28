#pragma once

#include "../GraphicsDefs.h"
#include <mutex>
#include <vector>
#include "VulkanDefs.h"

namespace mygfx {

	class PoolInfo;
	
	class DescriptorPoolManager {
	public:
		void init();
		void destroyAll();

		VkDescriptorPool& getPool(const DescriptorResourceCounts& counts, uint32_t totalSets = 64);
		void free(const VkDescriptorPool& pool, const DescriptorResourceCounts& counts);
	private:
		PoolInfo& createNewPool(const DescriptorResourceCounts& counts, uint32_t totalSets);
		std::vector<PoolInfo> pools_;
		std::mutex mutex_;
	};

	class PoolInfo {
	public:
		PoolInfo(const VkDescriptorPool& pool, const DescriptorResourceCounts& counts, uint32_t totalSets);

		bool allocate(const DescriptorResourceCounts& counts);
		void free(const DescriptorResourceCounts& counts);

		inline VkDescriptorPool& pool() { return pool_; }

	private:
		VkDescriptorPool pool_;
		DescriptorResourceCounts descriptorCounts{0};
		uint32_t remainingSets_;
	};

}

