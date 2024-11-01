#pragma once
#include "../GraphicsDefs.h"
#include "../GraphicsHandles.h"
#include "../PipelineState.h"
#include "../Uniforms.h"
#include "VulkanBuffer.h"
#include "VulkanDefs.h"
#include "VulkanHandles.h"
#include "VulkanShader.h"
#include "VulkanTextureView.h"

#include <span>

#define VULKAN_NOEXCEPT noexcept

#ifdef NDEBUG
#define VK_CHECK(__res) (__res)
#define VK_CHECK_MSG(__res, __msg) (__res)
#else
#define VK_CHECK(__res) assert(__res == VK_SUCCESS)
#define VK_CHECK_MSG(__res, __msg) assert(__res == VK_SUCCESS && __msg)
#endif

namespace mygfx {

class HwVertexInput;

struct RasterState;
struct ColorBlendState;
struct DepthState;
struct PipelineState;
struct StencilState;
class VulkanVertexInput;
class VulkanProgram;
class CommandList;
class CommandQueue;
class VulkanTexture;

enum class CommandQueueType {
    Graphics,
    Compute,
    Copy,
    Count
};

class CommandBuffer {
public:
    CommandBuffer(VkCommandBuffer c)
        : cmd(c)
    {
    }

    void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr) const VULKAN_NOEXCEPT;
    void begin(const VkCommandBufferBeginInfo& beginInfo) const VULKAN_NOEXCEPT;
    void end() const VULKAN_NOEXCEPT;

    void beginRendering(HwRenderTarget* pRT, const RenderPassInfo& renderInfo) const VULKAN_NOEXCEPT;
    void endRendering(HwRenderTarget* pRT) const VULKAN_NOEXCEPT;
    void resetState() const VULKAN_NOEXCEPT;
    void setViewportAndScissor(const VkRect2D& renderArea) const VULKAN_NOEXCEPT;
    void setViewportAndScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height) const VULKAN_NOEXCEPT;
    void setViewport(uint32_t viewportCount, const VkViewport* pViewports) const VULKAN_NOEXCEPT;
    void setScissor(uint32_t scissorCount, const VkRect2D* pScissors) const VULKAN_NOEXCEPT;
    void setVertexInput(HwVertexInput* vertexInput) const VULKAN_NOEXCEPT;
    void setPrimitiveTopology(PrimitiveTopology primitiveTopology) const VULKAN_NOEXCEPT;
    void setPrimitiveRestartEnable(bool restartEnable) const VULKAN_NOEXCEPT;
    void bindShaderProgram(HwProgram* program) const VULKAN_NOEXCEPT;
    void bindRasterState(const RasterState* rasterState) const VULKAN_NOEXCEPT;
    void bindColorBlendState(const ColorBlendState* colorBlendState) const VULKAN_NOEXCEPT;
    void bindDepthState(const DepthState* depthState) const VULKAN_NOEXCEPT;
    void bindStencilState(const StencilState* stencilState) const VULKAN_NOEXCEPT;
    void bindPipelineState(const PipelineState* pipelineState) const VULKAN_NOEXCEPT;
    void pushConstant(uint32_t binding, const void* data, uint32_t size) const VULKAN_NOEXCEPT;
    void pushConstant(uint32_t binding, const BufferInfo& bufferInfo) const VULKAN_NOEXCEPT;

    void bindDescriptorSets(HwDescriptorSet*const *, uint32_t setCount, const uint32_t* offsets, uint32_t offsetCount) const VULKAN_NOEXCEPT;

    void bindUniforms(const Uniforms& uniforms) const VULKAN_NOEXCEPT
    {
        bindUniformBuffer(uniforms.data(), uniforms.size());
    }

    void bindUniformBuffer(const uint32_t* offsets, uint32_t offsetCount) const VULKAN_NOEXCEPT;
    void bindIndexBuffer(HwBuffer* buffer, VkDeviceSize offset, IndexType indexType) const VULKAN_NOEXCEPT;
    void bindVertexBuffer(uint32_t firstBinding, HwBuffer* pBuffer, VkDeviceSize pOffsets = 0) const VULKAN_NOEXCEPT;
    void drawPrimitive(HwRenderPrimitive* primitive, uint32_t instanceCount = 1, uint32_t firstInstance = 0) const;
    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const VULKAN_NOEXCEPT;
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const VULKAN_NOEXCEPT;
    void drawIndirect(HwBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const VULKAN_NOEXCEPT;
    void drawIndexedIndirect(HwBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const VULKAN_NOEXCEPT;
    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const VULKAN_NOEXCEPT;
    void dispatchIndirect(HwBuffer* buffer, VkDeviceSize offset) const VULKAN_NOEXCEPT;
    void copyImage(VulkanTexture* srcTex, uint32_t srcLevel, uint32_t srcBaseLayer, VulkanTexture* destTex, uint32_t destLevel, uint32_t destBaseLayer) const VULKAN_NOEXCEPT;
    void resourceBarrier(uint32_t barrierCount, const Barrier* pBarriers) const VULKAN_NOEXCEPT;

    void setImageLayout(VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange) const VULKAN_NOEXCEPT;

    void pipelineBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        uint32_t memoryBarrierCount,
        const VkMemoryBarrier* pMemoryBarriers,
        uint32_t bufferMemoryBarrierCount,
        const VkBufferMemoryBarrier* pBufferMemoryBarriers,
        uint32_t imageMemoryBarrierCount,
        const VkImageMemoryBarrier* pImageMemoryBarriers) const VULKAN_NOEXCEPT;

    void pipelineBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        std::span<const VkMemoryBarrier> const& memoryBarriers,
        std::span<const VkBufferMemoryBarrier> const& bufferMemoryBarriers,
        std::span<const VkImageMemoryBarrier> const& imageMemoryBarriers) const VULKAN_NOEXCEPT;

    void free() const;

    CommandQueueType getCommandQueueType() const;

    VkCommandBuffer cmd;
    mutable CommandList* commandPool;

private:
    mutable VulkanVertexInput* mVertexInput;
    mutable VulkanProgram* mProgram;
    mutable PrimitiveState mPrimitiveState {};
    mutable VertexAttribute mVertexSemantic = VertexAttribute::ALL;
    mutable RasterState mRasterState {};
    mutable DepthState mDepthState {};
    mutable ColorBlendState mColorBlendState {};
    mutable StencilState mStencilState {};
    mutable HwRenderPrimitive* mPrimitive = nullptr;
};

inline void CommandBuffer::setVertexInput(HwVertexInput* vertexInput) const VULKAN_NOEXCEPT
{
    VulkanVertexInput* vkVertexInput = (VulkanVertexInput*)vertexInput;
    if (mVertexInput != vkVertexInput) {
        mVertexInput = vkVertexInput;

        g_vkCmdSetVertexInputEXT(cmd, (uint32_t)vkVertexInput->bindingDescriptions.size(), vkVertexInput->bindingDescriptions.data(),
            (uint32_t)vkVertexInput->attributeDescriptions.size(), vkVertexInput->attributeDescriptions.data());
    }
}

inline void CommandBuffer::setPrimitiveTopology(PrimitiveTopology primitiveTopology) const VULKAN_NOEXCEPT
{
    if (mPrimitiveState.primitiveTopology != primitiveTopology) {
        mPrimitiveState.primitiveTopology = primitiveTopology;

        g_vkCmdSetPrimitiveTopologyEXT(cmd, (VkPrimitiveTopology)primitiveTopology);
    }
}

inline void CommandBuffer::setPrimitiveRestartEnable(bool restartEnable) const VULKAN_NOEXCEPT
{
    if (mPrimitiveState.restartEnable != restartEnable) {
        mPrimitiveState.restartEnable = restartEnable;
        g_vkCmdSetPrimitiveRestartEnableEXT(cmd, restartEnable);
    }
}

inline void CommandBuffer::bindShaderProgram(HwProgram* program) const VULKAN_NOEXCEPT
{
    VulkanProgram* vkProgram = static_cast<VulkanProgram*>(program);
    if (mProgram != program) {
        mProgram = vkProgram;
        g_vkCmdBindShadersEXT(cmd, vkProgram->stageCount, vkProgram->stages, vkProgram->shaders);
    }
}

inline void CommandBuffer::pushConstant(uint32_t index, const void* data, uint32_t size) const VULKAN_NOEXCEPT
{
    if (index < mProgram->pushConstants.size()) {
        auto& pushConst = mProgram->pushConstants[index];
        assert(size <= pushConst.size);
        vkCmdPushConstants(cmd, mProgram->pipelineLayout, VK_SHADER_STAGE_ALL, pushConst.offset, size, data);
    }
}

inline void CommandBuffer::pushConstant(uint32_t binding, const BufferInfo& bufferInfo) const VULKAN_NOEXCEPT
{
    auto bufferAddr = bufferInfo.buffer->deviceAddress + bufferInfo.offset;
    vkCmdPushConstants(cmd, mProgram->pipelineLayout, VK_SHADER_STAGE_ALL, binding * 8, sizeof(uint64_t), &bufferAddr);
}

inline void CommandBuffer::bindUniformBuffer(const uint32_t* offsets, uint32_t offsetCount) const VULKAN_NOEXCEPT
{
    vkCmdBindDescriptorSets(cmd, mProgram->getBindPoint(), mProgram->pipelineLayout, 0,
        (uint32_t)mProgram->desciptorSets.size(), mProgram->desciptorSets.data(), offsetCount, offsets);
}

inline void CommandBuffer::bindIndexBuffer(HwBuffer* buffer, VkDeviceSize offset, IndexType indexType) const VULKAN_NOEXCEPT
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    vkCmdBindIndexBuffer(cmd, vkBuffer->buffer, static_cast<VkDeviceSize>(offset), (VkIndexType)indexType);
}

inline void CommandBuffer::bindVertexBuffer(uint32_t firstBinding, HwBuffer* pBuffer, VkDeviceSize pOffset) const VULKAN_NOEXCEPT
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(pBuffer);
    vkCmdBindVertexBuffers(cmd, firstBinding, 1, reinterpret_cast<const VkBuffer*>(&vkBuffer->buffer), reinterpret_cast<const VkDeviceSize*>(&pOffset));
}

inline void CommandBuffer::drawPrimitive(HwRenderPrimitive* primitive, uint32_t instanceCount, uint32_t firstInstance) const
{
    VulkanRenderPrimitive* rp = static_cast<VulkanRenderPrimitive*>(primitive);

    if (rp->indexBuffer != nullptr) {
        if (mPrimitive != primitive) {
            vkCmdBindVertexBuffers(cmd, 0, (uint32_t)rp->vertexBuffers.size(), rp->vertexBuffers.data(), rp->bufferOffsets.get());
            vkCmdBindIndexBuffer(cmd, rp->indexBuffer, 0, rp->indexType);
        }

        drawIndexed(rp->drawArgs.indexCount, instanceCount, rp->drawArgs.firstIndex, 0, firstInstance);

    } else {
        if (mPrimitive != primitive && rp->vertexBuffers.size() > 0) {
            vkCmdBindVertexBuffers(cmd, 0, (uint32_t)rp->vertexBuffers.size(), rp->vertexBuffers.data(), rp->bufferOffsets.get());
        }

        draw(rp->drawArgs.vertexCount, instanceCount, rp->drawArgs.firstVertex, firstInstance);
    }

    mPrimitive = primitive;
}

inline void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const VULKAN_NOEXCEPT
{
    vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void CommandBuffer::drawIndexed(uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance) const VULKAN_NOEXCEPT
{
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

inline void CommandBuffer::drawIndirect(HwBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const VULKAN_NOEXCEPT
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    vkCmdDrawIndirect(cmd, vkBuffer->buffer, offset, drawCount, stride);
}

inline void CommandBuffer::drawIndexedIndirect(HwBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const VULKAN_NOEXCEPT
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    vkCmdDrawIndexedIndirect(cmd, vkBuffer->buffer, offset, drawCount, stride);
}

inline void CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const VULKAN_NOEXCEPT
{
    vkCmdDispatch(cmd, groupCountX, groupCountY, groupCountZ);
}

inline void CommandBuffer::dispatchIndirect(HwBuffer* buffer, VkDeviceSize offset) const VULKAN_NOEXCEPT
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    vkCmdDispatchIndirect(cmd, vkBuffer->buffer, offset);
}

}