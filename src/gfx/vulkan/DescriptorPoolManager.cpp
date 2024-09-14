#include "DescriptorPoolManager.h"
#include "VulkanDevice.h"

namespace mygfx {

namespace {
    VkDescriptorType descriptorTypes[] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
        VK_DESCRIPTOR_TYPE_MUTABLE_VALVE,
        VK_DESCRIPTOR_TYPE_SAMPLER
    };
}

void DescriptorPoolManager::init()
{
}

VkDescriptorPool& DescriptorPoolManager::getPool(const DescriptorResourceCounts& counts, uint32_t totalSets)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& poolInfo : pools_) {
        if (poolInfo.allocate(counts)) {
            return poolInfo.pool();
        }
    }

    PoolInfo& newPool = createNewPool(counts, totalSets);
    bool result = newPool.allocate(counts);
    assert(result);
    return newPool.pool();
}

void DescriptorPoolManager::free(const VkDescriptorPool& pool, const DescriptorResourceCounts& counts)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& poolInfo : pools_) {
        if (poolInfo.pool() == pool) {
            poolInfo.free(counts);
        }
    }
}

PoolInfo& DescriptorPoolManager::createNewPool(const DescriptorResourceCounts& counts, uint32_t totalSets)
{
    std::vector<VkDescriptorPoolSize> sizes; //[DESCRIPTOR_TYPE_COUNT];
    for (int i = 0; i < DESCRIPTOR_TYPE_COUNT; i++) {
        if (counts[i] > 0) {
            sizes.push_back({ descriptorTypes[i], counts[i] });
        }
    }

    VkDescriptorPoolCreateInfo poolCI;
    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCI.pNext = nullptr;
    poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolCI.poolSizeCount = (uint32_t)sizes.size();
    poolCI.maxSets = totalSets;
    poolCI.pPoolSizes = sizes.data();

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(gfx().device, &poolCI, nullptr, &descriptorPool);
    return pools_.emplace_back(descriptorPool, counts, totalSets);
}

void DescriptorPoolManager::destroyAll()
{
    for (auto& poolInfo : pools_) {
        vkDestroyDescriptorPool(gfx().device, poolInfo.pool(), nullptr);
    }

    pools_.clear();
}

PoolInfo::PoolInfo(const VkDescriptorPool& pool, const DescriptorResourceCounts& counts, uint32_t totalSets)
{
    pool_ = pool;
    descriptorCounts = counts;
    remainingSets_ = totalSets;
}

bool PoolInfo::allocate(const DescriptorResourceCounts& counts)
{
    if (descriptorCounts != counts) {
        return false;
    }

    if (remainingSets_ <= 0) {
        return false;
    }

    remainingSets_ -= 1;

    return true;
}

void PoolInfo::free(const DescriptorResourceCounts& counts)
{
    remainingSets_ += 1;
}

}