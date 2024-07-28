#pragma once
#include "VulkanDefs.h"
#include "../GraphicsDefs.h"
#include "../GraphicsHandles.h"
#include "../PipelineState.h"
#include <span>

#define VULKAN_HPP_NOEXCEPT noexcept

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
    
	enum class CommandQueueType {
		Graphics,
		Compute,
		Copy,
		Count
	};

	class CommandBuffer {
	public:
		CommandBuffer(VkCommandBuffer c) : cmd(c) {}

        void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr) const;
		void begin(const VkCommandBufferBeginInfo& beginInfo) const;
		void end() const;

		void beginRendering(HwRenderTarget* pRT, const RenderPassInfo& renderInfo) const;
		void endRendering(HwRenderTarget* pRT) const;
		void resetState() const;
		void setViewportAndScissor(const VkRect2D& renderArea) const;
        void setViewportAndScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height) const;
		void setViewport(uint32_t viewportCount, const VkViewport* pViewports) const;
		void setScissor(uint32_t scissorCount, const VkRect2D* pScissors) const;
		void setVertexInput(HwVertexInput* vertexInput) const;
		void setPrimitiveTopology(PrimitiveTopology primitiveTopology) const;
		void setPrimitiveRestartEnable(bool restartEnable) const;
		void bindShaderProgram(HwProgram* program) const;
		void bindRasterState(const RasterState* rasterState) const;
		void bindColorBlendState(const ColorBlendState* colorBlendState) const;
		void bindDepthState(const DepthState* depthState) const;
		void bindStencilState(const StencilState* stencilState) const;
		void bindPipelineState(const PipelineState* pipelineState) const;
		void pushConstant(uint32_t binding, const BufferInfo& bufferInfo) const;

		void bindUniforms(const Uniforms& uniforms) const {
			bindUniformBuffer(uniforms.size(), uniforms.data());
		}

		void bindUniformBuffer(uint32_t offsetCount, const uint32_t* offsets) const;
        void bindIndexBuffer(HwBuffer* buffer, VkDeviceSize offset, IndexType indexType) const VULKAN_HPP_NOEXCEPT;
		void bindVertexBuffer(uint32_t firstBinding, HwBuffer* pBuffer, VkDeviceSize pOffsets = 0) const VULKAN_HPP_NOEXCEPT;		
		void drawPrimitive(HwRenderPrimitive* primitive) const;
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const VULKAN_HPP_NOEXCEPT;
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const VULKAN_HPP_NOEXCEPT;
        void drawIndirect(HwBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const VULKAN_HPP_NOEXCEPT;
        void drawIndexedIndirect(HwBuffer* buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const VULKAN_HPP_NOEXCEPT;
		void dispatch(uint32_t groupCountX,	uint32_t groupCountY, uint32_t groupCountZ) const VULKAN_HPP_NOEXCEPT;
        void dispatchIndirect(HwBuffer* buffer, VkDeviceSize offset) const VULKAN_HPP_NOEXCEPT;
		
		void resourceBarrier(uint32_t barrierCount, const Barrier* pBarriers) const;

		void setImageLayout(VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange) const VULKAN_HPP_NOEXCEPT;
		
        void pipelineBarrier(VkPipelineStageFlags          srcStageMask,
            VkPipelineStageFlags          dstStageMask,
            VkDependencyFlags             dependencyFlags,
            uint32_t                                          memoryBarrierCount,
            const VkMemoryBarrier* pMemoryBarriers,
            uint32_t                                          bufferMemoryBarrierCount,
            const VkBufferMemoryBarrier* pBufferMemoryBarriers,
            uint32_t                                          imageMemoryBarrierCount,
            const VkImageMemoryBarrier* pImageMemoryBarriers) const VULKAN_HPP_NOEXCEPT;

        void pipelineBarrier(VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags                dstStageMask,
            VkDependencyFlags                   dependencyFlags,
            std::span<const VkMemoryBarrier> const& memoryBarriers,
            std::span<const VkBufferMemoryBarrier> const& bufferMemoryBarriers,
            std::span<const VkImageMemoryBarrier> const& imageMemoryBarriers) const VULKAN_HPP_NOEXCEPT;

		void free() const;

		CommandQueueType getCommandQueueType() const;

		VkCommandBuffer cmd;
		mutable CommandList* commandPool;
	private:
		mutable VulkanVertexInput* mVertexInput;
		mutable VulkanProgram* mProgram;
		mutable PrimitiveState mPrimitiveState {};
		mutable VertexAttribute mVertexSemantic = VertexAttribute::All;
		mutable RasterState mRasterState {};
		mutable DepthState mDepthState {};
		mutable ColorBlendState mColorBlendState {};		
		mutable StencilState mStencilState {};
	};

}