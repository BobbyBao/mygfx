#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
// SRS - Enable beta extensions and make VK_KHR_portability_subset visible
#define VK_ENABLE_BETA_EXTENSIONS
#endif
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanHandles.h"
#include "VulkanStagePool.h"
#include "VulkanSwapchain.h"
#include "VulkanTextureView.h"
#include "api/CommandStreamDispatcher.h"
#include "utils/Log.h"
#include "vulkan/VulkanTexture.h"
#include <unordered_set>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#include "VkFormatHelper.h"
#include <vma/vk_mem_alloc.h>

namespace mygfx {

VulkanDevice& gfx()
{
    return static_cast<VulkanDevice&>(device());
}

GraphicsDevice* GraphicsDevice::createDevice()
{
    return new VulkanDevice();
}

VulkanDevice::VulkanDevice()
{
}

VulkanDevice::~VulkanDevice()
{
}

bool VulkanDevice::create(const Settings& settings)
{
    if (!VulkanDeviceHelper::create(settings.name, settings.validation)) {
        return false;
    }

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queue
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &mPresentComplete));

    mDescriptorPoolManager.init();

    mStagePool = new VulkanStagePool(mVmaAllocator);

    mTextureSet = new DescriptorTable(DescriptorType::COMBINED_IMAGE_SAMPLER | DescriptorType::STORAGE_IMAGE);
    // mImageSet = new DescriptorTable(DescriptorType::StorageImage);
    // mBufferSet = new DescriptorTable(DescriptorType::StorageBuffer);
    LOG_DEBUG("QueueFamilyIndex: {}, {}, {}", queueFamilyIndices.graphics, queueFamilyIndices.compute, queueFamilyIndices.transfer);
    mCommandQueues[0].init(CommandQueueType::Graphics, queueFamilyIndices.graphics, 0, 3, "");
    mCommandQueues[1].init(CommandQueueType::Compute, queueFamilyIndices.compute, 0, 3, "");
    mCommandQueues[2].init(CommandQueueType::Copy, queueFamilyIndices.transfer, copyQueueFamilyProperties().queueCount > 1 ? 1 : 0, 3, "");

    SamplerHandle::init();

    // Create a 'dynamic' constant buffer
    const uint32_t constantBuffersMemSize = 32 * 1024 * 1024;
    mConstantBufferRing.create(BufferUsage::UNIFORM | BufferUsage::STORAGE | BufferUsage::SHADER_DEVICE_ADDRESS,
        constantBuffersMemSize, MAX_BACKBUFFER_COUNT, constantBuffersMemSize, "Uniforms");
#if LARGE_DYNAMIC_INDEX
    const uint32_t vertexBuffersMemSize = 64 * 1024 * 1024;
#else
    const uint32_t vertexBuffersMemSize = 16 * 1024 * 1024;
#endif
    mVertexBufferRing.create(BufferUsage::VERTEX | BufferUsage::INDEX | BufferUsage::INDIRECT_BUFFER | BufferUsage::SHADER_DEVICE_ADDRESS, MAX_BACKBUFFER_COUNT, vertexBuffersMemSize, "VertexBuffers|IndexBuffers");

    const uint32_t uploadHeapMemSize = 64 * 1024 * 1024;
    mUploadHeap.create(uploadHeapMemSize);

    return true;
}

void* VulkanDevice::getInstanceData()
{
    return instance;
}

const char* VulkanDevice::getDeviceName() const
{
    return properties.deviceName;
}

void VulkanDevice::updateDynamicDescriptorSet(int index, uint32_t size, VkDescriptorSet descriptorSet)
{
    VulkanBuffer* vkBuffer = (VulkanBuffer*)mConstantBufferRing.getBuffer();
    VkDescriptorBufferInfo out = {};
    out.buffer = vkBuffer->buffer;
    out.offset = 0;
    out.range = size;

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = descriptorSet;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    write.pBufferInfo = &out;
    write.dstArrayElement = 0;
    write.dstBinding = index;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
}

CommandBuffer* VulkanDevice::getCommandBuffer(CommandQueueType queueType, uint32_t count)
{
    return mCommandQueues[(int)queueType].getCommandBuffer(count);
}

void VulkanDevice::freeCommandBuffer(CommandBuffer* cmd)
{
    mCommandQueues[(int)cmd->getCommandQueueType()].freeCommandBuffer(cmd);
}

void VulkanDevice::executeCommand(CommandQueueType queueType, const std::function<void(const CommandBuffer&)>& fn)
{
    auto cmd = getCommandBuffer(queueType);
    cmd->begin();
    fn(*cmd);
    cmd->end();
    uint64_t waitValue = mCommandQueues[(int)queueType].submit(&cmd->cmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE);
    mCommandQueues[(int)queueType].wait(waitValue);
    freeCommandBuffer(cmd);
}

void VulkanDevice::resize(HwSwapchain* sc, uint32_t destWidth, uint32_t destHeight)
{
    // Ensure all operations on the device have been finished before destroying resources
    vkDeviceWaitIdle(gfx().device);

    VulkanSwapChain* swapChain = static_cast<VulkanSwapChain*>(sc);

    // Recreate swap chain
    SwapChainDesc desc = sc->desc;
    desc.width = destWidth;
    desc.height = destHeight;
    swapChain->recreate(&desc);

    vkDeviceWaitIdle(device);
}

void VulkanDevice::destroy()
{
    // Flush device to make sure all resources can be freed
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);
    }

    HwObject::gc(true);

    // Clean up Vulkan resources
    mSwapChain.reset();

    vkDestroySemaphore(device, mPresentComplete, nullptr);

    mTextureSet.reset();

    mConstantBufferRing.destroy();
    mVertexBufferRing.destroy();

    mDescriptorPoolManager.destroyAll();

    SamplerHandle::shutdown();

    for (auto& s : mSamplers) {
        vkDestroySampler(device, s.second, nullptr);
    }

    mSamplers.clear();
    mStagePool->terminate();
    delete mStagePool;

    mCommandQueues[0].release();
    mCommandQueues[1].release();
    mCommandQueues[2].release();

    VulkanDeviceHelper::destroy();
}

bool VulkanDevice::allocConstantBuffer(uint32_t size, void** pData, BufferInfo* pOut)
{
    return mConstantBufferRing.allocBuffer(size, pData, pOut);
}

bool VulkanDevice::allocVertexBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void** pData, BufferInfo* pOut)
{
    return mVertexBufferRing.allocVertexBuffer(numbeOfVertices, strideInBytes, pData, pOut);
}

bool VulkanDevice::allocVertexBuffer(uint32_t sizeInBytes, void** pData, BufferInfo* pOut)
{
    return mVertexBufferRing.allocBuffer(sizeInBytes, pData, pOut);
}

bool VulkanDevice::allocIndexBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, void** pData, BufferInfo* pOut)
{
    return mVertexBufferRing.allocIndexBuffer(numbeOfIndices, strideInBytes, pData, pOut);
}

bool VulkanDevice::allocIndexBuffer(uint32_t sizeInBytes, void** pData, BufferInfo* pOut)
{
    return mVertexBufferRing.allocBuffer(sizeInBytes, pData, pOut);
}

Ref<HwBuffer> VulkanDevice::createBuffer(BufferUsage usage, MemoryUsage memoryUsage, uint64_t size, uint16_t stride, const void* data)
{
    return makeShared<VulkanBuffer>(usage, memoryUsage, size, stride, data);
}

Ref<HwTexture> VulkanDevice::createTexture(const TextureData& textureData, SamplerInfo sampler)
{
    return makeShared<VulkanTexture>(textureData, sampler);
}

Ref<HwTextureView> VulkanDevice::createSRV(HwTexture* tex, int mipLevel, const char* name)
{
    VulkanTexture* vkTex = static_cast<VulkanTexture*>(tex);
    return vkTex->createSRV(mipLevel);
}

Ref<HwTextureView> VulkanDevice::createRTV(HwTexture* tex, int mipLevel, const char* name)
{
    VulkanTexture* vkTex = static_cast<VulkanTexture*>(tex);
    return vkTex->createRTV(mipLevel);
}

bool VulkanDevice::copyData(HwTexture* tex, TextureDataProvider* dataProvider)
{
    VulkanTexture* vkTex = static_cast<VulkanTexture*>(tex);
    return vkTex->copyData(dataProvider);
}

VkSampler createVkSampler(const SamplerInfo& info)
{
    VkSamplerCreateInfo samplerCI = {};
    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCI.magFilter = (VkFilter)info.magFilter;
    samplerCI.minFilter = (VkFilter)info.minFilter;
    samplerCI.mipmapMode = (VkSamplerMipmapMode)info.mipmapMode;
    samplerCI.addressModeU = (VkSamplerAddressMode)info.addressModeU;
    samplerCI.addressModeV = (VkSamplerAddressMode)info.addressModeV;
    samplerCI.addressModeW = (VkSamplerAddressMode)info.addressModeW;

    samplerCI.mipLodBias = 0.0f;
    samplerCI.anisotropyEnable = info.anisotropyEnable;
    samplerCI.maxAnisotropy = 1.0f;
    samplerCI.compareEnable = info.compareEnable;
    samplerCI.compareOp = (VkCompareOp)info.compareOp;
    samplerCI.minLod = -1000;
    samplerCI.maxLod = 1000;

    samplerCI.borderColor = (VkBorderColor)info.borderColor;
    samplerCI.unnormalizedCoordinates = info.unnormalizedCoordinates;

    VkSampler vkSampler;
    vkCreateSampler(gfx().device, &samplerCI, nullptr, &vkSampler);
    return vkSampler;
}

SamplerHandle VulkanDevice::createSampler(const SamplerInfo& info)
{
    std::lock_guard<std::mutex> lock(mSamplerLock);
    for (int i = 0; i < mSamplers.size(); i++) {
        auto& s = mSamplers[i];
        if (s.first == info) {
            return SamplerHandle { .index = (uint16_t)i };
        }
    }
    auto s = createVkSampler(info);
    auto index = mSamplers.size();
    mSamplers.emplace_back(info, s);
    return SamplerHandle { .index = (uint16_t)index };
}

Ref<HwShaderModule> VulkanDevice::createShaderModule(ShaderStage stage, const ByteArray& shaderCode, ShaderCodeType shaderCodeType, const char* pShaderEntryPoint)
{
    return makeShared<VulkanShaderModule>(stage, shaderCode, shaderCodeType, pShaderEntryPoint);
}

SharedPtr<HwProgram> VulkanDevice::createProgram(Ref<HwShaderModule>* shaderModules, uint32_t count)
{
    SharedPtr<VulkanProgram> handle = makeShared<VulkanProgram>(shaderModules, count);
    return handle;
}

SharedPtr<HwRenderPrimitive> VulkanDevice::createRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive)
{
    return makeShared<VulkanRenderPrimitive>(geo, primitive);
}

SharedPtr<HwRenderTarget> VulkanDevice::createRenderTarget(const RenderTargetDesc& desc)
{
    return makeShared<VulkanRenderTarget>(desc);
}

SharedPtr<HwSwapchain> VulkanDevice::createSwapchain(const SwapChainDesc& desc)
{
    auto sw = makeShared<VulkanSwapChain>(desc);
    sw->recreate(&desc);
    return sw;
}

SharedPtr<HwVertexInput> VulkanDevice::createVertexInput(const FormatList& fmts, const FormatList& fmts1)
{
    return makeShared<VulkanVertexInput>(fmts, fmts1);
}

SharedPtr<HwDescriptorSet> VulkanDevice::createDescriptorSet(const Span<DescriptorSetLayoutBinding>& bindings)
{
    return makeShared<DescriptorSet>(bindings);
}

void VulkanDevice::updateTexture(HwTexture* texture,
    uint32_t level,
    uint32_t xoffset,
    uint32_t yoffset,
    uint32_t zoffset,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    const void* data,
    size_t size)
{
    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);
    vkTexture->setData(level, xoffset, yoffset, zoffset, width, height, depth, data, size);
}

void VulkanDevice::updateBuffer(HwBuffer* buffer, const void* data, size_t size, size_t offset)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    vkBuffer->setData(data, size, offset);
}

void VulkanDevice::copyTexture(HwTexture* srcTex, uint32_t srcLevel, uint32_t srcLayer,
    HwTexture* destTex, uint32_t destLevel, uint32_t destLayer)
{
    gfx().executeCommand(CommandQueueType::Copy, [=](auto& c) {
        c.copyImage((VulkanTexture*)srcTex, srcLevel, srcLayer, (VulkanTexture*)destTex, destLevel, destLayer);
    });
}

void VulkanDevice::updateDescriptorSet1(HwDescriptorSet* descriptorSet, uint32_t dstBinding, HwTextureView* texView)
{
    static_cast<DescriptorSet*>(descriptorSet)->bind(dstBinding, texView);
}

void VulkanDevice::updateDescriptorSet2(HwDescriptorSet* descriptorSet, uint32_t dstBinding, HwBuffer* buffer)
{
    static_cast<DescriptorSet*>(descriptorSet)->bind(dstBinding, buffer);
}

void VulkanDevice::updateDescriptorSet3(HwDescriptorSet* descriptorSet, uint32_t dstBinding, const BufferInfo& buffer)
{
    static_cast<DescriptorSet*>(descriptorSet)->bind(dstBinding, buffer);
}

void VulkanDevice::updateDescriptorSet4(HwDescriptorSet* descriptorSet, uint32_t dstBinding, uint32_t bufferSize)
{
    static_cast<DescriptorSet*>(descriptorSet)->bind(dstBinding, bufferSize);
}

void VulkanDevice::beginFrame(int)
{
    mCurrentCmd = getCommandBuffer(CommandQueueType::Graphics);
    mCurrentCmd->begin();
}

void VulkanDevice::beginRendering(HwRenderTarget* pRT, const RenderPassInfo& renderInfo)
{
    mCurrentCmd->beginRendering(pRT, renderInfo);
    mRenderPassInfo = renderInfo;
    mRenderTarget = (VulkanRenderTarget*)pRT;
    mAttachmentFormats.colorAttachmentCount = mRenderTarget->numAttachments();
    for (uint32_t i = 0; i < mAttachmentFormats.colorAttachmentCount; i++) {
        mAttachmentFormats.attachmentFormats[i] = mRenderTarget->colorAttachments[i]->format();
    }

    if (mRenderTarget->depthAttachment) {
        mAttachmentFormats.depthAttachmentFormat() = mRenderTarget->depthAttachment->format();
        if (isDepthStencilFormat(mAttachmentFormats.depthAttachmentFormat())) {
            mAttachmentFormats.stencilAttachmentFormat() = mAttachmentFormats.depthAttachmentFormat();
        } else {
            mAttachmentFormats.stencilAttachmentFormat() = VK_FORMAT_UNDEFINED;
        }

    } else {
        mAttachmentFormats.depthAttachmentFormat() = VK_FORMAT_UNDEFINED;
        mAttachmentFormats.stencilAttachmentFormat() = VK_FORMAT_UNDEFINED;
    }

    mAttachmentFormats.calculateHash();
}

void VulkanDevice::endRendering(HwRenderTarget* pRT)
{
    mCurrentCmd->endRendering(pRT);
    mRenderTarget = nullptr;
}

void VulkanDevice::makeCurrent(HwSwapchain* sc)
{
    mSwapChain = sc;

    VulkanSwapChain* swapChain = static_cast<VulkanSwapChain*>(sc);
    // Acquire the next image from the swap chain
    VkResult result = swapChain->acquireNextImage(mPresentComplete, &mCurrentImage, nullptr);
    swapChain->renderTarget->currentIndex = mCurrentImage;

    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
    // SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain images will change on resize
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            swapChain->recreate();
        }
        return;
    } else {
        VK_CHECK_RESULT(result);
    }
}

void VulkanDevice::resetState(int)
{
    mCurrentCmd->resetState();
}

void VulkanDevice::setViewport(float topX, float topY, float width, float height, float minDepth, float maxDepth)
{
    VkViewport viewport {
        .x = topX,
        .y = topY + height,
        .width = width,
        .height = -height,
        .minDepth = minDepth,
        .maxDepth = maxDepth
    };

    mCurrentCmd->setViewport(1, &viewport);
}

void VulkanDevice::setScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height)
{
    VkRect2D scissor {
        .offset = { (int32_t)topX, (int32_t)topY },
        .extent = { (uint32_t)(width), (uint32_t)(height) },
    };

    mCurrentCmd->setScissor(1, &scissor);
}

void VulkanDevice::setViewportAndScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height)
{
    mCurrentCmd->setViewportAndScissor(topX, topY, width, height);
}

void VulkanDevice::setVertexInput(HwVertexInput* vertexInput)
{
    mCurrentCmd->setVertexInput((VulkanVertexInput*)vertexInput);
}

void VulkanDevice::setPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
    mCurrentCmd->setPrimitiveTopology(primitiveTopology);
}

void VulkanDevice::setPrimitiveRestartEnable(bool restartEnable)
{
    mCurrentCmd->setPrimitiveRestartEnable(restartEnable);
}

void VulkanDevice::bindShaderProgram(HwProgram* program)
{
    mCurrentCmd->bindShaderProgram(program);
}

void VulkanDevice::bindRasterState(const RasterState& rasterState)
{
    mCurrentCmd->bindRasterState(&rasterState);
}

void VulkanDevice::bindColorBlendState(const ColorBlendState& colorBlendState)
{
    mCurrentCmd->bindColorBlendState(&colorBlendState);
}

void VulkanDevice::bindDepthState(const DepthState& depthState)
{
    mCurrentCmd->bindDepthState(&depthState);
}

void VulkanDevice::bindStencilState(const StencilState& stencilState)
{
    mCurrentCmd->bindStencilState(&stencilState);
}

void VulkanDevice::bindPipelineState(const PipelineState& pipelineState)
{
    mCurrentCmd->bindPipelineState(&pipelineState);
}

void VulkanDevice::pushConstant1(uint32_t index, const void* data, uint32_t size)
{
    mCurrentCmd->pushConstant(index, data, size);
}

void VulkanDevice::bindDescriptorSets1(const Span<HwDescriptorSet*>& ds, const Uniforms& uniforms)
{
    mCurrentCmd->bindDescriptorSets(ds.data(), (uint32_t)ds.size(), uniforms.data(), uniforms.size());
}

void VulkanDevice::bindUniforms(const Uniforms& uniforms)
{
    mCurrentCmd->bindUniformBuffer(uniforms.data(), uniforms.size());
}

void VulkanDevice::bindIndexBuffer(HwBuffer* buffer, uint64_t offset, IndexType indexType)
{
    mCurrentCmd->bindIndexBuffer(buffer, offset, indexType);
}

void VulkanDevice::bindVertexBuffer(uint32_t firstBinding, HwBuffer* buffer, uint64_t offset)
{
    mCurrentCmd->bindVertexBuffer(firstBinding, buffer, offset);
}

void VulkanDevice::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    mCurrentCmd->draw(vertexCount, instanceCount, firstVertex, firstInstance);
    Stats::drawCall()++;
}

void VulkanDevice::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    mCurrentCmd->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    Stats::drawCall()++;
}

void VulkanDevice::drawIndirect(HwBuffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
{
    mCurrentCmd->drawIndirect(buffer, offset, drawCount, stride);
}

void VulkanDevice::drawIndexedIndirect(HwBuffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
{
    mCurrentCmd->drawIndexedIndirect(buffer, offset, drawCount, stride);
}

void VulkanDevice::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    mCurrentCmd->dispatch(groupCountX, groupCountY, groupCountZ);
}

void VulkanDevice::dispatchIndirect(HwBuffer* buffer, uint64_t offset)
{
    mCurrentCmd->dispatchIndirect(buffer, offset);
}

void VulkanDevice::drawPrimitive(HwRenderPrimitive* primitive, uint32_t instanceCount, uint32_t firstInstance)
{
    VulkanRenderPrimitive* rp = static_cast<VulkanRenderPrimitive*>(primitive);
    if (rp->vertexBuffers.size() > 0) {
        vkCmdBindVertexBuffers(mCurrentCmd->cmd, 0, (uint32_t)rp->vertexBuffers.size(), rp->vertexBuffers.data(), rp->bufferOffsets.get());
    }

    if (rp->indexBuffer != nullptr) {
        vkCmdBindIndexBuffer(mCurrentCmd->cmd, rp->indexBuffer, 0, rp->indexType);
        mCurrentCmd->drawIndexed(rp->drawArgs.indexCount, instanceCount, rp->drawArgs.firstIndex, rp->drawArgs.vertexOffset, firstInstance);
    } else {
        mCurrentCmd->draw(rp->drawArgs.vertexCount, instanceCount, rp->drawArgs.firstVertex, firstInstance);
    }

    Stats::drawCall()++;
}

void VulkanDevice::drawIndirectPrimitive(HwRenderPrimitive* primitive, HwBuffer* indirectBuffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
{
    VulkanRenderPrimitive* rp = static_cast<VulkanRenderPrimitive*>(primitive);
    if (rp->vertexBuffers.size() > 0) {
        vkCmdBindVertexBuffers(mCurrentCmd->cmd, 0, (uint32_t)rp->vertexBuffers.size(), rp->vertexBuffers.data(), rp->bufferOffsets.get());
    }

    if (rp->indexBuffer != nullptr) {
        vkCmdBindIndexBuffer(mCurrentCmd->cmd, rp->indexBuffer, 0, rp->indexType);
        mCurrentCmd->drawIndexedIndirect(indirectBuffer, offset, drawCount, stride);
    } else {
        mCurrentCmd->drawIndirect(indirectBuffer, offset, drawCount, stride);
    }

    Stats::drawCall()++;
}

static void drawBatch1(const CommandBuffer& cmd, const RenderCommand* start, uint32_t count) VULKAN_NOEXCEPT
{
    for (uint32_t i = 0; i < count; i++) {
        auto& primitive = start[i];
        cmd.bindPipelineState(&primitive.pipelineState);
        cmd.bindUniforms(primitive.uniforms);
        cmd.drawPrimitive(primitive.renderPrimitive, primitive.instanceCount, 0);
    }
}

void VulkanDevice::drawBatch(HwRenderQueue* renderQueue)
{
    const auto& primitives = renderQueue->getReadCommands();
    // #if HAS_SHADER_OBJECT_EXT
    if (primitives.size() > 200) {
        drawMultiThreaded(primitives, *mCurrentCmd);
    } else {
        drawBatch1(*mCurrentCmd, primitives.data(), (uint32_t)primitives.size());
    }
    // #else
    //     drawBatch1(*mCurrentCmd, primitives.data(), (uint32_t)primitives.size());
    // #endif

    Stats::drawCall() += (uint32_t)primitives.size();
}

void drawWork(VulkanDevice* device, const RenderCommand* start, uint32_t count, uint32_t index)
{
    CommandBuffer& cb = *device->mCmdList[index];

    VkCommandBufferInheritanceRenderingInfoKHR info1 = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO_KHR,
        .pNext = nullptr,
        .flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT,
        .viewMask = 0,
        .colorAttachmentCount = device->mAttachmentFormats.colorAttachmentCount,
        .pColorAttachmentFormats = device->mAttachmentFormats.attachmentFormats,
        .depthAttachmentFormat = device->mAttachmentFormats.depthAttachmentFormat(),
        .stencilAttachmentFormat = device->mAttachmentFormats.stencilAttachmentFormat(),
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkCommandBufferInheritanceInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .pNext = &info1,
    };

    cb.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, &info);
    auto& vp = device->mRenderPassInfo.viewport;
    cb.setViewportAndScissor(vp.left, vp.top, vp.width, vp.height);
    cb.resetState();
    drawBatch1(cb, start, count);

    cb.end();
}

void VulkanDevice::drawMultiThreaded(const std::vector<RenderCommand>& items, const CommandBuffer& cmd)
{
    uint32_t itemsPerThread = 200;
    uint32_t threadNum = std::thread::hardware_concurrency();
    itemsPerThread = (uint32_t)(items.size() + threadNum - 1) / threadNum;
    itemsPerThread = std::max(40u, itemsPerThread);

    mFutures.clear();
    uint32_t start = 0, end = 0;

    mSecondCmdBuffers.reserve(threadNum);

#if !HAS_SHADER_OBJECT_EXT
    for (auto& prim : items) {
        VulkanProgram* vkProgram = (VulkanProgram*)prim.pipelineState.program;
        vkProgram->getGraphicsPipeline(mAttachmentFormats, &prim.pipelineState);
    }
#endif

    for (unsigned i = 0; i < threadNum && start < items.size(); ++i) {
        auto cmdList = mCommandQueues[(int)CommandQueueType::Graphics].getCommandBuffer(1, true);
        mCmdList.push_back(cmdList);
        mSecondCmdBuffers.push_back(cmdList->cmd);
        uint32_t itemCount = std::min(itemsPerThread, (uint32_t)items.size() - start);
        mFutures.emplace_back(std::async(std::launch::async,
            drawWork, this, (const RenderCommand*)&items[start], (uint32_t)itemCount, (uint32_t)i));

        start += itemCount;
    }

    for (auto& f : mFutures) {
        f.get();
    }

    vkCmdExecuteCommands(cmd.cmd, (uint32_t)mSecondCmdBuffers.size(), mSecondCmdBuffers.data());
    mSecondCmdBuffers.clear();

    for (auto cmdList : mCmdList) {
        auto c = cmdList;
        post([=]() {
            mCommandQueues[(int)CommandQueueType::Graphics].releaseCommandPool(c->commandPool);
            c->free();
        },
            4);
    }

    mCmdList.clear();
}

void VulkanDevice::resourceBarrier(uint32_t barrierCount, const Barrier* pBarriers)
{
    mCurrentCmd->resourceBarrier(barrierCount, pBarriers);
}

void VulkanDevice::commit(HwSwapchain* sc)
{
    mCurrentCmd->end();

    assert(sc == mSwapChain);
    VulkanSwapChain* swapChain = static_cast<VulkanSwapChain*>(sc);

    auto semaphoreValue = mCommandQueues[0].submit(&mCurrentCmd->cmd, 1, nullptr, mPresentComplete, mCurrentImage);
    mCommandQueues[0].present(swapChain->swapChain, mCurrentImage);
    auto cmd = (CommandBuffer*)mCurrentCmd;
    auto future = std::async(std::launch::async, [this, semaphoreValue, cmd]() {
        mCommandQueues[0].wait(semaphoreValue);
        freeCommandBuffer(cmd);
    });

    mCurrentCmd = nullptr;
}

void VulkanDevice::endFrame(int)
{
    HwObject::gc();
    mStagePool->gc();
}

Dispatcher VulkanDevice::getDispatcher() const noexcept
{
    return ConcreteDispatcher<VulkanDevice>::make();
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<VulkanDevice>;
}
