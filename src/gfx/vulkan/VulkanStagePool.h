#pragma once

#include "../GraphicsDefs.h"
#include "../utils/SpinLock.h"
#include "VulkanDefs.h"
#include <map>
#include <unordered_set>

namespace mygfx {

// Immutable POD representing a shared CPU-GPU staging area.
struct VulkanStage {
    VmaAllocation memory;
    VkBuffer buffer;
    size_t capacity;
    mutable uint64_t lastAccessed;
};

struct VulkanStageImage {
    VkFormat format;
    uint32_t width;
    uint32_t height;
    mutable uint64_t lastAccessed;
    VmaAllocation memory;
    VkImage image;
};

// Manages a pool of stages, periodically releasing stages that have been unused for a while.
// This class manages two types of host-mappable staging areas: buffer stages and image stages.
class VulkanStagePool {
public:
    VulkanStagePool(VmaAllocator allocator);

    // Finds or creates a stage whose capacity is at least the given number of bytes.
    // The stage is automatically released back to the pool after TIME_BEFORE_EVICTION frames.
    VulkanStage const* acquireStage(const void* buffer, size_t size);

    // Images have VK_IMAGE_LAYOUT_GENERAL and must not be transitioned to any other layout
    VulkanStageImage const* acquireImage(Format format, uint32_t width, uint32_t height);

    // Evicts old unused stages and bumps the current frame number.
    void gc() noexcept;

    // Destroys all unused stages and asserts that there are no stages currently in use.
    // This should be called while the context's VkDevice is still alive.
    void terminate() noexcept;

private:
    VmaAllocator mAllocator;

    utils::SpinLock mLockFreeStages;
    // Use an ordered multimap for quick (capacity => stage) lookups using lower_bound().
    std::multimap<size_t, VulkanStage const*> mFreeStages;

    utils::SpinLock mLockUsedStages;
    // Simple unordered set for stashing a list of in-use stages that can be reclaimed later.
    std::unordered_set<VulkanStage const*> mUsedStages;

    utils::SpinLock mLockFreeImages;
    utils::SpinLock mLockUsedImages;
    std::unordered_set<VulkanStageImage const*> mFreeImages;
    std::unordered_set<VulkanStageImage const*> mUsedImages;

    // Store the current "time" (really just a frame count) and LRU eviction parameters.
    uint64_t mCurrentFrame = 0;
};

}
