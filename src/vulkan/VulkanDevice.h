#pragma once
#include "../GraphicsDevice.h"
#include "VulkanBuffer.h"
#include "VulkanDebug.h"
#include "VulkanDefs.h"
#include "VulkanHelper.h"
#include "VulkanInitializers.hpp"
#include "VulkanStagePool.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanTools.h"
#include "CommandQueue.h"
#include "DescriptorPoolManager.h"
#include "ResourceSet.h"
#include "UploadHeap.h"
#include <algorithm>
#include <assert.h>
#include <deque>
#include <exception>
#include <future>

namespace mygfx {
class VulkanBuffer;
class VulkanRenderTarget;
class VulkanStagePool;

class VulkanDevice : public GraphicsDevice, public VulkanHelper {
public:
    VulkanDevice();
    ~VulkanDevice();

    bool create(const Settings& settings) override;

    const char* getDeviceName() const override;
    Dispatcher getDispatcher() const noexcept override;

    VmaAllocator vmaAllocator() { return vmaAllocator_; }
    DynamicBufferPool& getConstbufferRing() { return mConstantBufferRing; }

    bool allocVertexBuffer(uint32_t sizeInBytes, void** pData, BufferInfo* pOut);
    bool allocIndexBuffer(uint32_t sizeInBytes, void** pData, BufferInfo* pOut);

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##S() noexcept override;                          \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "api/GraphicsAPI.inc"

    void destroy();

    DescriptorPoolManager& descriptorPools()
    {
        return mDescriptorPoolManager;
    }

    VulkanStagePool& getStagePool() { return *mStagePool; }
    UploadHeap& uploadHeap() { return mUploadHeap; }
    DescriptorTable* getTextureSet() { return mTextureSet; }
    DescriptorTable* getImageSet() { return mImageSet; }
    DescriptorTable* getBufferSet() { return mBufferSet; }

    VkBuffer constBuffer()
    {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(mConstantBufferRing.getBuffer());
        return vkBuffer->buffer;
    }

    void updateDynamicDescriptorSet(int index, uint32_t size, VkDescriptorSet descriptorSet);

    CommandBuffer* getCommandBuffer(CommandQueueType queueType, uint32_t count = 1);
    void freeCommandBuffer(CommandBuffer* cmd);
    void executeCommand(CommandQueueType queueType, const std::function<void(const CommandBuffer&)>& fn);

    VkSampler getVkSampler(SamplerHandle sampler)
    {
        return mSamplers[sampler.index].second;
    }

protected:
    void createCommandPool();
    void createSynchronizationPrimitives();
    void createCommandBuffers();
    void destroyCommandBuffers();

    const CommandBuffer* getFree()
    {
        auto ret = freeCmdBuffers.front();
        freeCmdBuffers.pop_front();
        return ret;
    }

    void drawMultiThreaded(const std::vector<RenderCommand>& items, const CommandBuffer& cmd);

    // Command buffer pool
    VkCommandPool cmdPool { VK_NULL_HANDLE };
    /** @brief Pipeline stages used to wait at for graphics queue submissions */
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // Contains command buffers and semaphores to be presented to the queue
    VkSubmitInfo submitInfo;

    // Synchronization semaphores
    struct {
        // Swap chain image presentation
        VkSemaphore presentComplete;
        // Command buffer submission and execution
        VkSemaphore renderComplete;
    } semaphores;
    std::vector<VkFence> waitFences;

    VmaAllocator vmaAllocator_ = NULL;

    DynamicBufferPool mConstantBufferRing;
    DynamicBufferPool mVertexBufferRing;
    UploadHeap mUploadHeap;
    std::mutex mSamplerLock;
    std::vector<std::pair<SamplerInfo, VkSampler>> mSamplers;
    DescriptorPoolManager mDescriptorPoolManager;

    Ref<DescriptorTable> mTextureSet;
    Ref<DescriptorTable> mImageSet;
    Ref<DescriptorTable> mBufferSet;

    CommandQueue mCommandQueues[(int)CommandQueueType::Count];

    std::vector<VkCommandBuffer> cmdBuffers;
    std::vector<CommandBuffer> drawCmdBuffers;

    std::vector<const CommandBuffer*> submitCmdBuffers[4];
    std::deque<const CommandBuffer*> freeCmdBuffers;
    // Active frame buffer index
    uint32_t currentBuffer = 0;
    const CommandBuffer* currentCmd = nullptr;

    std::vector<std::future<void>> mFutures {};
    std::vector<VkCommandBuffer> mSecondCmdBuffers;

    RenderPassInfo mRenderPassInfo {};
    VulkanRenderTarget* mRenderTarget = nullptr;
    std::vector<CommandBuffer*> mCmdList;
    uint32_t colorAttachmentCount;
    VkFormat colorAttachmentFormats[8];
    VkFormat depthAttachmentFormat;
    VkFormat stencilAttachmentFormat;
    VulkanStagePool* mStagePool = nullptr;

    friend class CommandBuffer;

    friend void drawWork(VulkanDevice* device, const RenderCommand* start, uint32_t count, uint32_t index);
};

VulkanDevice& gfx();

} // namespace mygfx
