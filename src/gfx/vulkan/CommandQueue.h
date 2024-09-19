#pragma once
#include "../GraphicsDefs.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "VulkanDefs.h"
#include "utils/concurrentqueue.h"

namespace mygfx {

class CommandQueue {
public:
    CommandQueue() = default;

    void init(CommandQueueType queueType, uint32_t queueFamilyIndex, uint32_t queueIndex, uint32_t numFramesInFlight, const char* name);
    void release();

    CommandBuffer* getCommandBuffer(uint32_t count, bool isSecond = false);
    void freeCommandBuffer(CommandBuffer* cmd);

    CommandList* getCommandPool();
    void releaseCommandPool(CommandList* commandPool);

    // thread safe
    uint64_t submit(const std::vector<CommandBuffer>& cmdLists, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, int useEndOfFrameSemaphore = -1);
    uint64_t submit(const VkCommandBuffer* cmdLists, uint32_t count, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, int useEndOfFrameSemaphore = -1);
    uint64_t present(VkSwapchainKHR swapchain, uint32_t imageIndex); // only valid on the present queue
    void wait(uint64_t waitValue) const;

    void flush();

    VkSemaphore getOwnershipTransferSemaphore();
    void releaseOwnershipTransferSemaphore(VkSemaphore semaphore);

protected:
    VkQueue mQueue = VK_NULL_HANDLE;
    CommandQueueType mQueueType;
    VkSemaphore mSemaphore;
    uint64_t mLatestSemaphoreValue = 0;
    uint32_t mFamilyIndex;
    moodycamel::ConcurrentQueue<CommandList*> mAvailableCommandPools;
    std::vector<VkSemaphore> mFrameSemaphores = {};
    std::vector<VkSemaphore> mAvailableOwnershipTransferSemaphores = {};
    std::vector<VkSemaphore> mUsedOwnershipTransferSemaphores = {};
    std::mutex mSubmitMutex;
};

}
