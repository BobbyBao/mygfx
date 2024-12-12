#include "CommandQueue.h"
#include "VulkanDevice.h"

namespace mygfx {

void CommandQueue::init(CommandQueueType queueType, uint32_t queueFamilyIndex, uint32_t queueIndex, uint32_t numFramesInFlight, const char* name)
{
    mQueueType = queueType;
    mFamilyIndex = queueFamilyIndex;
    vkGetDeviceQueue(gfx().device, queueFamilyIndex, queueIndex, &mQueue);
    gfx().setResourceName(VK_OBJECT_TYPE_QUEUE, (uint64_t)mQueue, name);

    // create timeline semaphore
    VkSemaphoreTypeCreateInfo typeCreateInfo = {};
    typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    typeCreateInfo.pNext = nullptr;
    typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    typeCreateInfo.initialValue = mLatestSemaphoreValue;

    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = &typeCreateInfo;
    createInfo.flags = 0;
    VkResult res = vkCreateSemaphore(gfx().device, &createInfo, nullptr, &mSemaphore);
    // Assert(ASSERT_CRITICAL, res == VK_SUCCESS, "Failed to create queue semaphore!");

    gfx().setResourceName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)mSemaphore, "PhantomTimelineSemaphore");

    // create the frame semaphores semaphores
    createInfo.pNext = nullptr;
    createInfo.flags = 0; // not signaled
    mFrameSemaphores.reserve(numFramesInFlight);
    for (uint32_t i = 0; i < numFramesInFlight; ++i) {
        VkSemaphore semaphore = VK_NULL_HANDLE;
        res = vkCreateSemaphore(gfx().device, &createInfo, nullptr, &semaphore);
        // Assert(ASSERT_CRITICAL, res == VK_SUCCESS && semaphore != VK_NULL_HANDLE, "Failed to create queue semaphore!");

        gfx().setResourceName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)semaphore, "PhantomSemaphore");
        mFrameSemaphores.push_back(semaphore);
    }
}

void CommandQueue::release()
{
    CommandList* commandPool = nullptr;
    while (mAvailableCommandPools.try_dequeue(commandPool)) {
        vkDestroyCommandPool(gfx().device, commandPool->handle(), nullptr);
    }

    vkDestroySemaphore(gfx().device, mSemaphore, nullptr);

    for (size_t i = 0; i < mFrameSemaphores.size(); ++i) {
        vkDestroySemaphore(gfx().device, mFrameSemaphores[i], nullptr);
    }
    mFrameSemaphores.clear();
}

CommandBuffer* CommandQueue::getCommandBuffer(uint32_t count, bool isSecond)
{
    auto pool = getCommandPool();
    return pool->alloc(count, isSecond);
}

void CommandQueue::freeCommandBuffer(CommandBuffer* cmd)
{   
    auto pool = cmd->commandPool;
    cmd->free();
    releaseCommandPool(pool);
}

CommandList* CommandQueue::getCommandPool()
{
    CommandList* pool = nullptr;

    // Check if there are any available allocators we can use
    if (mAvailableCommandPools.try_dequeue(pool)) {
        // reset the pool before using it
        vkResetCommandPool(gfx().device, pool->handle(), VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        return pool;
    }
    // No available allocators, so create a new one
    else {
        pool = new CommandList(mFamilyIndex);
        pool->commandQueueType = mQueueType;
    }
    return pool;
}

void CommandQueue::releaseCommandPool(CommandList* commandPool)
{
    mAvailableCommandPools.enqueue(commandPool);
}

uint64_t CommandQueue::submit(const std::vector<CommandBuffer>& cmdLists, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, int useEndOfFrameSemaphore)
{
    std::lock_guard<std::mutex> lock(mSubmitMutex);

    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.reserve(cmdLists.size());
    for (auto& list : cmdLists) {
        // Assert(ASSERT_CRITICAL, m_QueueType == list->GetQueueType(), "Command list is submitted on the wrong queue.");
        commandBuffers.push_back(list.cmd);
    }

    return submit(commandBuffers.data(), (uint32_t)commandBuffers.size(), signalSemaphore, waitSemaphore, useEndOfFrameSemaphore);
}

uint64_t CommandQueue::submit(const VkCommandBuffer* commandBuffers, uint32_t count, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, int useEndOfFrameSemaphore)
{
    VkPipelineStageFlags waitStageFlags[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
    VkSemaphore waitSemaphores[] = { mSemaphore, waitSemaphore };

    uint32_t signalSemaphoreCount = 0;
    VkSemaphore signalSemaphores[3];
    signalSemaphores[signalSemaphoreCount++] = mSemaphore;
    if (signalSemaphore != VK_NULL_HANDLE)
        signalSemaphores[signalSemaphoreCount++] = signalSemaphore;
    if (useEndOfFrameSemaphore >= 0)
        signalSemaphores[signalSemaphoreCount++] = mFrameSemaphores[useEndOfFrameSemaphore];

    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.waitSemaphoreCount = (waitSemaphore == VK_NULL_HANDLE) ? 1 : 2;
    info.pWaitSemaphores = waitSemaphores;
    info.pWaitDstStageMask = waitStageFlags;
    info.pCommandBuffers = commandBuffers;
    info.commandBufferCount = count;
    info.signalSemaphoreCount = signalSemaphoreCount;
    info.pSignalSemaphores = signalSemaphores;

    uint64_t semaphoreWaitValues[] = { mLatestSemaphoreValue, 0 };
    ++mLatestSemaphoreValue;

    // the second value is useless
    uint64_t semaphoreSignalValues[] = { mLatestSemaphoreValue, 0, 0 };

    VkTimelineSemaphoreSubmitInfo semaphoreSubmitInfo = {};
    semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    semaphoreSubmitInfo.pNext = nullptr;
    semaphoreSubmitInfo.waitSemaphoreValueCount = info.waitSemaphoreCount;
    semaphoreSubmitInfo.pWaitSemaphoreValues = semaphoreWaitValues;
    semaphoreSubmitInfo.signalSemaphoreValueCount = info.signalSemaphoreCount;
    semaphoreSubmitInfo.pSignalSemaphoreValues = semaphoreSignalValues;

    info.pNext = &semaphoreSubmitInfo;

    std::lock_guard<std::mutex> lock(mSubmitMutex);
    vkQueueSubmit(mQueue, 1, &info, VK_NULL_HANDLE);
    // LOG_DEBUG("vkQueueSubmit : {}, {}", (uint64_t)m_Queue, std::this_thread::get_id()._Get_underlying_id());
    return mLatestSemaphoreValue;
}

uint64_t CommandQueue::present(VkSwapchainKHR swapchain, uint32_t imageIndex) // only valid on the present queue
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mFrameSemaphores[imageIndex]; // NOTE: imageIndex is technically different from the frame in flight index but we are using the same in Cauldron.
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    std::lock_guard<std::mutex> lock(mSubmitMutex);

    VkResult res = vkQueuePresentKHR(mQueue, &presentInfo);
    // Assert(ASSERT_ERROR, res == VK_SUCCESS, "Failed to present");

    return mLatestSemaphoreValue;
}

void CommandQueue::wait(uint64_t waitValue) const
{
    VkSemaphoreWaitInfo waitInfo = {};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.pNext = nullptr;
    waitInfo.flags = 0;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &mSemaphore;
    waitInfo.pValues = &waitValue;

#if VK_VERSION == VK_1_1
    VkResult res = vkWaitSemaphoresKHR(gfx().device, &waitInfo, std::numeric_limits<uint64_t>::max());
#else
    VkResult res = vkWaitSemaphores(gfx().device, &waitInfo, std::numeric_limits<uint64_t>::max());
#endif
    // LOG_DEBUG("                         vkWaitSemaphores : {}, {}", (uint64_t)m_Queue, std::this_thread::get_id()._Get_underlying_id());
    // Assert(ASSERT_WARNING, res == VK_SUCCESS, "Failed to wait on the queue semaphore.");
}

void CommandQueue::flush()
{
    std::lock_guard<std::mutex> lock(mSubmitMutex);
    vkQueueWaitIdle(mQueue);
}

VkSemaphore CommandQueue::getOwnershipTransferSemaphore()
{
    std::lock_guard<std::mutex> lock(mSubmitMutex);

    VkSemaphore semaphore;

    if (mAvailableOwnershipTransferSemaphores.size() > 0) {
        semaphore = mAvailableOwnershipTransferSemaphores[mAvailableOwnershipTransferSemaphores.size() - 1];
        mAvailableOwnershipTransferSemaphores.pop_back();
    } else {
        VkSemaphoreCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        VkResult res = vkCreateSemaphore(gfx().device, &createInfo, nullptr, &semaphore);
        // Assert(ASSERT_CRITICAL, res == VK_SUCCESS, "Failed to create queue ownership transfer semaphore!");
        gfx().setResourceName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)semaphore, "PhantomOwnershipTransferSemaphore");
    }

    mUsedOwnershipTransferSemaphores.push_back(semaphore);

    return semaphore;
}

void CommandQueue::releaseOwnershipTransferSemaphore(VkSemaphore semaphore)
{
    std::lock_guard<std::mutex> lock(mSubmitMutex);

    for (auto it = mUsedOwnershipTransferSemaphores.begin(); it != mUsedOwnershipTransferSemaphores.end(); ++it) {
        if (*it == semaphore) {
            mUsedOwnershipTransferSemaphores.erase(it);
            mAvailableOwnershipTransferSemaphores.push_back(semaphore);
            return;
        }
    }

    // Critical("Queue ownership transfer semaphore to release wasn't found.");
}

}