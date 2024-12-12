#include "VulkanStagePool.h"
#include "VulkanImageUtility.h"

// #include <utils/Panic.h>

static constexpr uint32_t TIME_BEFORE_EVICTION = 10;

namespace mygfx {

VulkanStagePool::VulkanStagePool(VmaAllocator allocator)
    : mAllocator(allocator)
{
}

VulkanStage const* VulkanStagePool::acquireStage(const void* buffer, size_t size)
{
    mLockFreeStages.lock();

    // First check if a stage exists whose capacity is greater than or equal to the requested size.
    auto iter = mFreeStages.lower_bound(size);
    if (iter != mFreeStages.end()) {
        auto stage = iter->second;
        mFreeStages.erase(iter);
        utils::ScopedSpinLock lock(mLockUsedStages);
        mUsedStages.insert(stage);
        mLockFreeStages.unlock();
        stage->lastAccessed = mCurrentFrame;

        void* mapped = nullptr;
        assert(stage->memory);
        vmaMapMemory(mAllocator, stage->memory, &mapped);
        memcpy(mapped, buffer, size);
        vmaUnmapMemory(mAllocator, stage->memory);
        vmaFlushAllocation(mAllocator, stage->memory, 0, size);

        return stage;
    }
    mLockFreeStages.unlock();

    // We were not able to find a sufficiently large stage, so create a new one.
    VulkanStage* stage = new VulkanStage({
        .memory = VK_NULL_HANDLE,
        .buffer = VK_NULL_HANDLE,
        .capacity = size,
        .lastAccessed = mCurrentFrame,
    });

    // Create the VkBuffer.
    mLockUsedStages.lock();
    mUsedStages.insert(stage);
    mLockUsedStages.unlock();

    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_CPU_ONLY };
    UTILS_UNUSED_IN_RELEASE VkResult result = vmaCreateBuffer(mAllocator, &bufferInfo,
        &allocInfo, &stage->buffer, &stage->memory, nullptr);

    if (result != VK_SUCCESS) {
        LOG_ERROR("Allocation error: {}", (int)result);
    }

    void* mapped = nullptr;
    assert(stage->memory);
    vmaMapMemory(mAllocator, stage->memory, &mapped);
    memcpy(mapped, buffer, size);
    vmaUnmapMemory(mAllocator, stage->memory);
    vmaFlushAllocation(mAllocator, stage->memory, 0, size);

    return stage;
}

VulkanStageImage const* VulkanStagePool::acquireImage(Format format, uint32_t width, uint32_t height)
{
    const VkFormat vkformat = imgutil::toVk(format);
    for (auto image : mFreeImages) {
        if (image->format == vkformat && image->width == width && image->height == height) {
            mFreeImages.erase(image);
            mUsedImages.insert(image);
            image->lastAccessed = mCurrentFrame;
            return image;
        }
    }

    VulkanStageImage* image = new VulkanStageImage({
        .format = vkformat,
        .width = width,
        .height = height,
        .lastAccessed = mCurrentFrame,
    });

    mUsedImages.insert(image);

    const VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = vkformat,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    };

    const VmaAllocationCreateInfo allocInfo {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    const UTILS_UNUSED VkResult result = vmaCreateImage(mAllocator, &imageInfo, &allocInfo,
        &image->image, &image->memory, nullptr);

    assert(result == VK_SUCCESS);

    VkImageAspectFlags const aspectFlags = imgutil::getAspectFlags(vkformat);
    VkCommandBuffer const cmdbuffer = nullptr; // mCommands->get().buffer();

    // We use VK_IMAGE_LAYOUT_GENERAL here because the spec says:
    // "Host access to image memory is only well-defined for linear images and for image
    // subresources of those images which are currently in either the
    // VK_IMAGE_LAYOUT_PREINITIALIZED or VK_IMAGE_LAYOUT_GENERAL layout. Calling
    // vkGetImageSubresourceLayout for a linear image returns a subresource layout mapping that is
    // valid for either of those image layouts."
    imgutil::transitionLayout(cmdbuffer, {
                                             .image = image->image,
                                             .oldLayout = VulkanLayout::UNDEFINED,
                                             .newLayout = VulkanLayout::READ_WRITE, // (= VK_IMAGE_LAYOUT_GENERAL)
                                             .subresources = { aspectFlags, 0, 1, 0, 1 },
                                         });
    return image;
}

void VulkanStagePool::gc() noexcept
{
    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    if (++mCurrentFrame <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;

    mLockFreeStages.lock();
    // Destroy buffers that have not been used for several frames.
    decltype(mFreeStages) freeStages;
    freeStages.swap(mFreeStages);
    for (auto pair : freeStages) {
        if (pair.second->lastAccessed < evictionTime) {
            vmaDestroyBuffer(mAllocator, pair.second->buffer, pair.second->memory);
            delete pair.second;
        } else {
            mFreeStages.insert(pair);
        }
    }

    {
        utils::ScopedSpinLock lock(mLockUsedStages);
        // Reclaim buffers that are no longer being used by any command buffer.
        decltype(mUsedStages) usedStages;
        usedStages.swap(mUsedStages);
        for (auto stage : usedStages) {
            if (stage->lastAccessed < evictionTime) {
                stage->lastAccessed = mCurrentFrame;
                mFreeStages.insert(std::make_pair(stage->capacity, stage));
            } else {
                mUsedStages.insert(stage);
            }
        }
    }

    mLockFreeStages.unlock();

    // Destroy images that have not been used for several frames.
    decltype(mFreeImages) freeImages;
    freeImages.swap(mFreeImages);
    for (auto image : freeImages) {
        if (image->lastAccessed < evictionTime) {
            vmaDestroyImage(mAllocator, image->image, image->memory);
            delete image;
        } else {
            mFreeImages.insert(image);
        }
    }

    // Reclaim images that are no longer being used by any command buffer.
    decltype(mUsedImages) usedImages;
    usedImages.swap(mUsedImages);
    for (auto image : usedImages) {
        if (image->lastAccessed < evictionTime) {
            image->lastAccessed = mCurrentFrame;
            mFreeImages.insert(image);
        } else {
            mUsedImages.insert(image);
        }
    }
}

void VulkanStagePool::terminate() noexcept
{
    for (auto stage : mUsedStages) {
        vmaDestroyBuffer(mAllocator, stage->buffer, stage->memory);
        delete stage;
    }
    mUsedStages.clear();

    for (auto pair : mFreeStages) {
        vmaDestroyBuffer(mAllocator, pair.second->buffer, pair.second->memory);
        delete pair.second;
    }
    mFreeStages.clear();

    for (auto image : mUsedImages) {
        vmaDestroyImage(mAllocator, image->image, image->memory);
        delete image;
    }
    mUsedStages.clear();

    for (auto image : mFreeImages) {
        vmaDestroyImage(mAllocator, image->image, image->memory);
        delete image;
    }
    mFreeStages.clear();
}

}
