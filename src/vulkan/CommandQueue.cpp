#include "CommandQueue.h"
#include "VulkanDevice.h"

namespace mygfx
{
	
    void CommandQueue::Init(CommandQueueType queueType, uint32_t queueFamilyIndex, uint32_t queueIndex, uint32_t numFramesInFlight, const char* name)
    {
        m_QueueType = queueType;
        m_FamilyIndex = queueFamilyIndex;
        vkGetDeviceQueue(gfx().device, queueFamilyIndex, queueIndex, &m_Queue);
        gfx().setResourceName(VK_OBJECT_TYPE_QUEUE, (uint64_t)m_Queue, name);

        // create timeline semaphore
        VkSemaphoreTypeCreateInfo typeCreateInfo = {};
        typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        typeCreateInfo.pNext = nullptr;
        typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        typeCreateInfo.initialValue = m_LatestSemaphoreValue;

        VkSemaphoreCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = &typeCreateInfo;
        createInfo.flags = 0;
        VkResult res = vkCreateSemaphore(gfx().device, &createInfo, nullptr, &m_Semaphore);
        //Assert(ASSERT_CRITICAL, res == VK_SUCCESS, "Failed to create queue semaphore!");

        gfx().setResourceName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_Semaphore, "PhantomTimelineSemaphore");

        // create the frame semaphores semaphores
        createInfo.pNext = nullptr;
        createInfo.flags = 0; // not signaled
        m_FrameSemaphores.reserve(numFramesInFlight);
        for (uint32_t i = 0; i < numFramesInFlight; ++i)
        {
            VkSemaphore semaphore = VK_NULL_HANDLE;
            res = vkCreateSemaphore(gfx().device, &createInfo, nullptr, &semaphore);
            //Assert(ASSERT_CRITICAL, res == VK_SUCCESS && semaphore != VK_NULL_HANDLE, "Failed to create queue semaphore!");

            gfx().setResourceName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)semaphore, "PhantomSemaphore");
            m_FrameSemaphores.push_back(semaphore);
        }
    }

    void CommandQueue::Release(VkDevice device)
    {
        CommandList* commandPool = nullptr;
        while (m_AvailableCommandPools.try_dequeue(commandPool))
        {
            vkDestroyCommandPool(device, commandPool->handle(), nullptr);
        }

        vkDestroySemaphore(device, m_Semaphore, nullptr);

        for (size_t i = 0; i < m_FrameSemaphores.size(); ++i)
        {
            vkDestroySemaphore(device, m_FrameSemaphores[i], nullptr);
        }
        m_FrameSemaphores.clear();
    }
    
    CommandBuffer* CommandQueue::getCommandBuffer(uint32_t count, bool isSecond) {
        auto pool = GetCommandPool();
        return pool->alloc(count, isSecond);
    }
    
    void CommandQueue::freeCommandBuffer(CommandBuffer* cmd) {
        ReleaseCommandPool(cmd->commandPool);
        cmd->free();
    }

    CommandList* CommandQueue::GetCommandPool()
    {
        CommandList* pool = nullptr;

        // Check if there are any available allocators we can use
        if (m_AvailableCommandPools.try_dequeue(pool))
        {
            // reset the pool before using it
            vkResetCommandPool(gfx().device, pool->handle(), VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

            return pool;
        }
        // No available allocators, so create a new one
        else
        {
            pool = new CommandList(m_FamilyIndex);
            pool->commandQueueType = m_QueueType;
        }
        return pool;
    }

    void CommandQueue::ReleaseCommandPool(CommandList* commandPool)
    {
        m_AvailableCommandPools.enqueue(commandPool);
    }

    uint64_t CommandQueue::Submit(const std::vector<CommandBuffer>& cmdLists, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, bool useEndOfFrameSemaphore)
    {
        std::lock_guard<std::mutex> lock(m_SubmitMutex);

        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(cmdLists.size());
        for (auto& list : cmdLists)
        {
            //Assert(ASSERT_CRITICAL, m_QueueType == list->GetQueueType(), "Command list is submitted on the wrong queue.");
            commandBuffers.push_back(list.cmd);
        }

        return Submit(commandBuffers.data(), (uint32_t)commandBuffers.size(), signalSemaphore, waitSemaphore, useEndOfFrameSemaphore);
    }
       
    uint64_t CommandQueue::Submit(const VkCommandBuffer* commandBuffers, uint32_t count, const VkSemaphore signalSemaphore, const VkSemaphore waitSemaphore, bool useEndOfFrameSemaphore)
    {
        VkPipelineStageFlags waitStageFlags[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
        VkSemaphore waitSemaphores[] = { m_Semaphore, waitSemaphore };

        uint32_t signalSemaphoreCount = 0;
        VkSemaphore signalSemaphores[3];
        signalSemaphores[signalSemaphoreCount++] = m_Semaphore;
        if (signalSemaphore != VK_NULL_HANDLE)
            signalSemaphores[signalSemaphoreCount++] = signalSemaphore;
        //if (useEndOfFrameSemaphore)
        //    signalSemaphores[signalSemaphoreCount++] = m_FrameSemaphores[GetSwapChain()->GetBackBufferIndex()];

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

        uint64_t semaphoreWaitValues[] = { m_LatestSemaphoreValue, 0 };
        ++m_LatestSemaphoreValue;

        // the second value is useless
        uint64_t semaphoreSignalValues[] = { m_LatestSemaphoreValue, 0, 0 };

        VkTimelineSemaphoreSubmitInfo semaphoreSubmitInfo = {};
        semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSubmitInfo.pNext = nullptr;
        semaphoreSubmitInfo.waitSemaphoreValueCount = info.waitSemaphoreCount;
        semaphoreSubmitInfo.pWaitSemaphoreValues = semaphoreWaitValues;
        semaphoreSubmitInfo.signalSemaphoreValueCount = info.signalSemaphoreCount;
        semaphoreSubmitInfo.pSignalSemaphoreValues = semaphoreSignalValues;

        info.pNext = &semaphoreSubmitInfo;
        
        vkQueueSubmit(m_Queue, 1, &info, VK_NULL_HANDLE);
        //LOG_DEBUG("vkQueueSubmit : {}, {}", (uint64_t)m_Queue, std::this_thread::get_id()._Get_underlying_id());
        return m_LatestSemaphoreValue;
    }

    uint64_t CommandQueue::Present(VkSwapchainKHR swapchain, uint32_t imageIndex) // only valid on the present queue
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_FrameSemaphores[imageIndex]; // NOTE: imageIndex is technically different from the frame in flight index but we are using the same in Cauldron.
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        std::lock_guard<std::mutex> lock(m_SubmitMutex);

        VkResult res = vkQueuePresentKHR(m_Queue, &presentInfo);
        //Assert(ASSERT_ERROR, res == VK_SUCCESS, "Failed to present");

        return m_LatestSemaphoreValue;
    }

    void CommandQueue::Wait(VkDevice device, uint64_t waitValue) const
    {
        VkSemaphoreWaitInfo waitInfo = {};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.pNext = nullptr;
        waitInfo.flags = 0;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &m_Semaphore;
        waitInfo.pValues = &waitValue;
        VkResult res = vkWaitSemaphores(device, &waitInfo, std::numeric_limits<uint64_t>::max());
        
        //LOG_DEBUG("                         vkWaitSemaphores : {}, {}", (uint64_t)m_Queue, std::this_thread::get_id()._Get_underlying_id());
        //Assert(ASSERT_WARNING, res == VK_SUCCESS, "Failed to wait on the queue semaphore.");
    }

    void CommandQueue::Flush()
    {
        std::lock_guard<std::mutex> lock(m_SubmitMutex);
        vkQueueWaitIdle(m_Queue);
    }

    VkSemaphore CommandQueue::GetOwnershipTransferSemaphore()
    {
        std::lock_guard<std::mutex> lock(m_SubmitMutex);

        //DeviceInternal* pDevice = device()->GetImpl();

        VkSemaphore semaphore;

        if (m_AvailableOwnershipTransferSemaphores.size() > 0)
        {
            semaphore = m_AvailableOwnershipTransferSemaphores[m_AvailableOwnershipTransferSemaphores.size() - 1];
            m_AvailableOwnershipTransferSemaphores.pop_back();
        }
        else
        {
            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;

            VkResult res = vkCreateSemaphore(gfx().device, &createInfo, nullptr, &semaphore);
            //Assert(ASSERT_CRITICAL, res == VK_SUCCESS, "Failed to create queue ownership transfer semaphore!");
            gfx().setResourceName(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)semaphore, "PhantomOwnershipTransferSemaphore");
        }

        m_UsedOwnershipTransferSemaphores.push_back(semaphore);

        return semaphore;
    }

    void CommandQueue::ReleaseOwnershipTransferSemaphore(VkSemaphore semaphore)
    {
        std::lock_guard<std::mutex> lock(m_SubmitMutex);

        for (auto it = m_UsedOwnershipTransferSemaphores.begin(); it != m_UsedOwnershipTransferSemaphores.end(); ++it)
        {
            if (*it == semaphore)
            {
                m_UsedOwnershipTransferSemaphores.erase(it);
                m_AvailableOwnershipTransferSemaphores.push_back(semaphore);
                return;
            }
        }

        //Critical("Queue ownership transfer semaphore to release wasn't found.");
    }

}