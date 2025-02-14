#pragma once

#include "../GraphicsDefs.h"
#include "VulkanDefs.h"
#include <mutex>
#include <vector>

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
    std::vector<PoolInfo> mPools;
    std::mutex mMutex;
};

class PoolInfo {
public:
    PoolInfo(const VkDescriptorPool& pool, const DescriptorResourceCounts& counts, uint32_t totalSets);

    bool allocate(const DescriptorResourceCounts& counts);
    void free(const DescriptorResourceCounts& counts);

    inline VkDescriptorPool& pool() { return mPool; }

private:
    VkDescriptorPool mPool;
    DescriptorResourceCounts mDescriptorCounts { 0 };
    uint32_t mRemainingSets;
};

}
