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

    void Init(CommandQueueType queueType, uint32_t queueFamilyIndex, uint32_t queueIndex, uint32_t numFramesInFlight, const char* name);
    void Release(VkDevice device);

    CommandBuffer* getCommandBuffer(uint32_t count, bool isSecond = false);
    void freeCommandBuffer(CommandBuffer* cmd);

    CommandList* GetCommandPool();
    void ReleaseCommandPool(CommandList* commandPool);

    // thread safe
    uint64_t submit(const std::vector<CommandBuffer>& cmdLists, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, int useEndOfFrameSemaphore = -1);
    uint64_t submit(const VkCommandBuffer* cmdLists, uint32_t count, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, int useEndOfFrameSemaphore = -1);
    uint64_t present(VkSwapchainKHR swapchain, uint32_t imageIndex); // only valid on the present queue
    void Wait(VkDevice device, uint64_t waitValue) const;

    void Flush();

    VkSemaphore GetOwnershipTransferSemaphore();
    void ReleaseOwnershipTransferSemaphore(VkSemaphore semaphore);

protected:
    VkQueue m_Queue = VK_NULL_HANDLE;
    CommandQueueType m_QueueType;
    VkSemaphore m_Semaphore;
    uint64_t m_LatestSemaphoreValue = 0;
    uint32_t m_FamilyIndex;

    moodycamel::ConcurrentQueue<CommandList*> m_AvailableCommandPools;
    std::vector<VkSemaphore> m_FrameSemaphores = {};

    std::vector<VkSemaphore> m_AvailableOwnershipTransferSemaphores = {};
    std::vector<VkSemaphore> m_UsedOwnershipTransferSemaphores = {};

    std::mutex m_SubmitMutex;
};

}
