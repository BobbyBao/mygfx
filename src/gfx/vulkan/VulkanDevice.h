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

    VmaAllocator getVmaAllocator() { return mVmaAllocator; }
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

    DescriptorPoolManager& getDescriptorPools()
    {
        return mDescriptorPoolManager;
    }

    VulkanStagePool& getStagePool() { return *mStagePool; }
    UploadHeap& getUploadHeap() { return mUploadHeap; }
    DescriptorTable* getTextureSet() { return mTextureSet; }
    DescriptorTable* getImageSet() { return mImageSet; }
    DescriptorTable* getBufferSet() { return mBufferSet; }

    VkBuffer getGlobalUniformBuffer()
    {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(mConstantBufferRing.getBuffer());
        return vkBuffer->buffer;
    }
    
    VkBuffer getGlobalDynamicBuffer()
    {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(mVertexBufferRing.getBuffer());
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
    void drawMultiThreaded(const std::vector<RenderCommand>& items, const CommandBuffer& cmd);

    VmaAllocator mVmaAllocator = NULL;

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

    VulkanStagePool* mStagePool = nullptr;
    // Swap chain image presentation
    VkSemaphore mPresentComplete;
    
    // Active frame buffer index
    uint32_t mCurrentImage = 0;
    const CommandBuffer* mCurrentCmd = nullptr;

    std::vector<std::future<void>> mFutures {};
    std::vector<VkCommandBuffer> mSecondCmdBuffers;
    std::vector<CommandBuffer*> mCmdList;

    RenderPassInfo mRenderPassInfo {};
    VulkanRenderTarget* mRenderTarget = nullptr;
    AttachmentFormats mAttachmentFormats;

    friend class CommandBuffer;

    friend void drawWork(VulkanDevice* device, const RenderCommand* start, uint32_t count, uint32_t index);
};

VulkanDevice& gfx();

} // namespace mygfx
