#include "CommandBuffer.h"
#include "CommandPool.h"
#include "VulkanDevice.h"
#include "VulkanImageUtility.h"

namespace mygfx {

void CommandBuffer::begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo) const VULKAN_NOEXCEPT
{
    VkCommandBufferBeginInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = 0,
        .flags = flags,
        .pInheritanceInfo = pInheritanceInfo
    };

    begin(info);
}

void CommandBuffer::begin(const VkCommandBufferBeginInfo& beginInfo) const VULKAN_NOEXCEPT
{
    VkResult result = vkBeginCommandBuffer(cmd, &beginInfo);
    VK_CHECK_MSG(result, "CommandBuffer::begin"); 
}

void CommandBuffer::end() const VULKAN_NOEXCEPT
{
    VkResult result = vkEndCommandBuffer(cmd);
    VK_CHECK_MSG(result, "CommandBuffer::end");
}

void CommandBuffer::beginRendering(HwRenderTarget* pRT, const RenderPassInfo& renderInfo) const VULKAN_NOEXCEPT
{
    VulkanRenderTarget* pVkRT = (VulkanRenderTarget*)pRT;

    // Transition color and depth images for drawing
    if (pVkRT->isSwapchain) {
        tools::insertImageMemoryBarrier(
            cmd,
            pVkRT->colorAttachments[pVkRT->currentIndex]->image(),
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkImageSubresourceRange { VK_IMAGE_ASPECT_COLOR_BIT, 0,
                1, 0, 1 });
    } else {
        // todo:check
        for (auto& t : pVkRT->colorAttachments) {
            tools::insertImageMemoryBarrier(
                cmd,
                t->image(),
                0,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VkImageSubresourceRange { VK_IMAGE_ASPECT_COLOR_BIT, 0,
                    1, 0, 1 });
        }
    }

    bool hasStencil = true;
    if (pVkRT->depthAttachment) {
        auto& fmtInfo = getFormatInfo(imgutil::fromVk(pVkRT->depthAttachment->format()));
        auto dstLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        if (fmtInfo.depth && !fmtInfo.stencil) {
            dstLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            hasStencil = false;
        } else if (fmtInfo.stencil && !fmtInfo.depth) {
            dstLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        }

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE;
        if (fmtInfo.depth) {
            aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        if (fmtInfo.stencil) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        tools::insertImageMemoryBarrier(
            cmd,
            pVkRT->depthAttachment->image(),
            0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            dstLayout,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VkImageSubresourceRange { aspectMask, 0, 1, 0, 1 });
    }

    // New structures are used to define the attachments used in dynamic rendering
    VkRenderingAttachmentInfoKHR colorAttachment[8] {};
    if (pVkRT->isSwapchain) {
        colorAttachment[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachment[0].imageView = pVkRT->colorAttachments[pVkRT->currentIndex]->handle();
        colorAttachment[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment[0].loadOp = any(renderInfo.clearFlags & TargetBufferFlags::COLOR_0) ? VK_ATTACHMENT_LOAD_OP_CLEAR
            : any(renderInfo.loadFlags & TargetBufferFlags::COLOR_0)                        ? VK_ATTACHMENT_LOAD_OP_LOAD
                                                                                            : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment[0].storeOp = any(renderInfo.storeFlags & TargetBufferFlags::COLOR_0) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        std::memcpy(&colorAttachment[0].clearValue.color, renderInfo.clearColor, sizeof(float) * 4);
    } else {
        // todo:
        for (size_t i = 0; i < pVkRT->colorAttachments.size(); i++) {
            colorAttachment[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            colorAttachment[i].imageView = pVkRT->colorAttachments[i]->handle();
            colorAttachment[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachment[i].loadOp = any(renderInfo.clearFlags & TargetBufferFlags(1 << i)) ? VK_ATTACHMENT_LOAD_OP_CLEAR
                : any(renderInfo.loadFlags & TargetBufferFlags(1 << i))                        ? VK_ATTACHMENT_LOAD_OP_LOAD
                                                                                               : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment[i].storeOp = any(renderInfo.storeFlags & TargetBufferFlags(1 << i)) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            std::memcpy(&colorAttachment[i].clearValue.color, renderInfo.clearColor, sizeof(float) * 4);
        }
    }
    // A single depth stencil attachment info can be used, but they can also be specified separately.
    // When both are specified separately, the only requirement is that the image view is identical.
    VkRenderingAttachmentInfoKHR depthStencilAttachment {};

    if (pVkRT->depthAttachment) {
        auto& fmtInfo = getFormatInfo(imgutil::fromVk(pVkRT->depthAttachment->format()));
        auto dstLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        if (fmtInfo.depth && !fmtInfo.stencil) {
            dstLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            hasStencil = false;
        } else if (fmtInfo.stencil && !fmtInfo.depth) {
            dstLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        }
        depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depthStencilAttachment.imageView = pVkRT->depthAttachment->handle();
        depthStencilAttachment.imageLayout = dstLayout;
        depthStencilAttachment.loadOp = any(renderInfo.clearFlags & TargetBufferFlags::DEPTH) ? VK_ATTACHMENT_LOAD_OP_CLEAR
            : any(renderInfo.loadFlags & TargetBufferFlags::DEPTH)                            ? VK_ATTACHMENT_LOAD_OP_LOAD
                                                                                              : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachment.storeOp = any(renderInfo.storeFlags & TargetBufferFlags::DEPTH) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachment.clearValue.depthStencil = { InvertedDepth ? 0.0f : 1.0f, 0 };
    }

    VkRenderingInfoKHR renderingInfo {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    renderingInfo.renderArea = {
        renderInfo.viewport.left,
        renderInfo.viewport.top,
        std::min(renderInfo.viewport.width, pVkRT->width),
        std::min(renderInfo.viewport.height, pVkRT->height)
    };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = (uint32_t)pVkRT->numAttachments();
    renderingInfo.pColorAttachments = &colorAttachment[0];

    if (pVkRT->depthAttachment) {
        renderingInfo.pDepthAttachment = &depthStencilAttachment;
        renderingInfo.pStencilAttachment = hasStencil ? &depthStencilAttachment : nullptr;
    }

    // Begin dynamic rendering
    g_vkCmdBeginRenderingKHR(cmd, &renderingInfo);

    setViewportAndScissor(renderInfo.viewport.left, renderInfo.viewport.top,
        std::min(renderInfo.viewport.width, pVkRT->width),
        std::min(renderInfo.viewport.height, pVkRT->height));

    resetState();
}

void CommandBuffer::endRendering(HwRenderTarget* pRT) const VULKAN_NOEXCEPT
{
    // End dynamic rendering
    g_vkCmdEndRenderingKHR(cmd);

    VulkanRenderTarget* pVkRT = (VulkanRenderTarget*)pRT;
    if (pVkRT->isSwapchain) {

        // Transition color image for presentation
        tools::insertImageMemoryBarrier(
            cmd,
            pVkRT->colorAttachments[pVkRT->currentIndex]->image(),
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VkImageSubresourceRange { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    } else {
        // todo:check
        for (auto& t : pVkRT->colorAttachments) {
            tools::insertImageMemoryBarrier(
                cmd,
                t->image(),
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                0,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VkImageSubresourceRange { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
        }
    }
}

void CommandBuffer::resetState() const VULKAN_NOEXCEPT
{
    g_vkCmdSetCullModeEXT(cmd, VK_CULL_MODE_BACK_BIT);
    g_vkCmdSetFrontFaceEXT(cmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    g_vkCmdSetDepthTestEnableEXT(cmd, VK_TRUE);
    g_vkCmdSetDepthWriteEnableEXT(cmd, VK_TRUE);
    g_vkCmdSetDepthCompareOpEXT(cmd, InvertedDepth ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_LESS_OR_EQUAL);
    g_vkCmdSetPrimitiveTopologyEXT(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    g_vkCmdSetRasterizerDiscardEnableEXT(cmd, VK_FALSE);
    g_vkCmdSetPolygonModeEXT(cmd, VK_POLYGON_MODE_FILL);
    g_vkCmdSetRasterizationSamplesEXT(cmd, VK_SAMPLE_COUNT_1_BIT);
    g_vkCmdSetAlphaToCoverageEnableEXT(cmd, VK_FALSE);
    g_vkCmdSetDepthBiasEnableEXT(cmd, VK_FALSE);
    g_vkCmdSetStencilTestEnableEXT(cmd, VK_FALSE);
    g_vkCmdSetPrimitiveRestartEnableEXT(cmd, VK_FALSE);

    const uint32_t sampleMask = 0xFF;
    g_vkCmdSetSampleMaskEXT(cmd, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

    const VkBool32 colorBlendEnables = false;
    const VkColorComponentFlags colorBlendComponentFlags = 0xf;
    const VkColorBlendEquationEXT colorBlendEquation {};
    g_vkCmdSetColorBlendEnableEXT(cmd, 0, 1, &colorBlendEnables);
    g_vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorBlendComponentFlags);

    mProgram = nullptr;
    mVertexInput = nullptr;
    mPrimitiveState = {};
    mVertexSemantic = VertexAttribute::ALL;
    mRasterState = {};
    mDepthState = {};
    mColorBlendState = {};
    mStencilState = {};
    mPrimitive = nullptr;
}

void CommandBuffer::setViewportAndScissor(const VkRect2D& renderArea) const VULKAN_NOEXCEPT
{
    setViewportAndScissor(renderArea.offset.x, renderArea.offset.y, renderArea.extent.width, renderArea.extent.height);
}

void CommandBuffer::setViewportAndScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height) const VULKAN_NOEXCEPT
{
    VkViewport viewport;
    viewport.x = static_cast<float>(topX);
    viewport.y = static_cast<float>(topY) + static_cast<float>(height);
    viewport.width = static_cast<float>(width);
    viewport.height = -static_cast<float>(height);
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    g_vkCmdSetViewportWithCountEXT(cmd, 1, &viewport);

    VkRect2D scissor;
    scissor.extent.width = (uint32_t)(width);
    scissor.extent.height = (uint32_t)(height);
    scissor.offset.x = topX;
    scissor.offset.y = topY;

    g_vkCmdSetScissorWithCountEXT(cmd, 1, &scissor);
}

void CommandBuffer::setViewport(uint32_t viewportCount, const VkViewport* pViewports) const VULKAN_NOEXCEPT
{
    g_vkCmdSetViewportWithCountEXT(cmd, viewportCount, reinterpret_cast<const VkViewport*>(pViewports));
}

void CommandBuffer::setScissor(uint32_t scissorCount, const VkRect2D* pScissors) const VULKAN_NOEXCEPT
{
    g_vkCmdSetScissorWithCountEXT(cmd, scissorCount, reinterpret_cast<const VkRect2D*>(pScissors));
}

void CommandBuffer::bindRasterState(const RasterState* rasterState) const VULKAN_NOEXCEPT
{
    if (mRasterState == *rasterState) {
        return;
    }

    mRasterState = *rasterState;

    g_vkCmdSetPolygonModeEXT(cmd, (VkPolygonMode)rasterState->polygonMode);
    g_vkCmdSetCullModeEXT(cmd, (VkCullModeFlags)rasterState->cullMode);
    g_vkCmdSetFrontFaceEXT(cmd, (VkFrontFace)rasterState->frontFace);
    g_vkCmdSetRasterizerDiscardEnableEXT(cmd, rasterState->rasterizerDiscardEnable);
    g_vkCmdSetRasterizationSamplesEXT(cmd, (VkSampleCountFlagBits)rasterState->rasterizationSamples);

    const uint32_t sampleMask = 0xFF;
    g_vkCmdSetSampleMaskEXT(cmd, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

    g_vkCmdSetAlphaToCoverageEnableEXT(cmd, rasterState->alphaToCoverageEnable);
    g_vkCmdSetDepthBiasEnableEXT(cmd, rasterState->depthBiasEnable);
}

void CommandBuffer::bindColorBlendState(const ColorBlendState* colorBlendState) const VULKAN_NOEXCEPT
{

    if (mColorBlendState == *colorBlendState) {
        return;
    }

    mColorBlendState = *colorBlendState;

    const VkBool32 colorBlendEnables = colorBlendState->colorBlendEnable;
    const VkColorComponentFlags colorBlendComponentFlags = (VkColorComponentFlags)colorBlendState->colorWrite;
    const VkColorBlendEquationEXT colorBlendEquation {
        .srcColorBlendFactor = (VkBlendFactor)colorBlendState->srcColorBlendFactor,
        .dstColorBlendFactor = (VkBlendFactor)colorBlendState->dstColorBlendFactor,
        .colorBlendOp = (VkBlendOp)colorBlendState->colorBlendOp,
        .srcAlphaBlendFactor = (VkBlendFactor)colorBlendState->srcAlphaBlendFactor,
        .dstAlphaBlendFactor = (VkBlendFactor)colorBlendState->dstAlphaBlendFactor,
        .alphaBlendOp = (VkBlendOp)colorBlendState->alphaBlendOp,
    };

    g_vkCmdSetColorBlendEnableEXT(cmd, 0, 1, &colorBlendEnables);
    g_vkCmdSetColorBlendEquationEXT(cmd, 0, 1, &colorBlendEquation);
    g_vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorBlendComponentFlags);
}

VkCompareOp convertComparisonFunc(const CompareOp func)
{
    static bool invertedDepth = InvertedDepth;

    switch (func) {
    case CompareOp::NEVER:
        return VK_COMPARE_OP_NEVER;
    case CompareOp::LESS:
        return invertedDepth ? VK_COMPARE_OP_GREATER : VK_COMPARE_OP_LESS;
    case CompareOp::EQUAL:
        return VK_COMPARE_OP_EQUAL;
    case CompareOp::LESS_OR_EQUAL:
        return invertedDepth ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::GREATER:
        return invertedDepth ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_GREATER;
    case CompareOp::NOT_EQUAL:
        return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::GREATER_OR_EQUAL:
        return invertedDepth ? VK_COMPARE_OP_LESS_OR_EQUAL : VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::ALWAYS:
        return VK_COMPARE_OP_ALWAYS;
    default:
        return VK_COMPARE_OP_NEVER;
    }
}

void CommandBuffer::bindDepthState(const DepthState* depthState) const VULKAN_NOEXCEPT
{
    if (mDepthState == *depthState) {
        return;
    }

    mDepthState = *depthState;

    g_vkCmdSetDepthTestEnableEXT(cmd, depthState->depthTestEnable);
    g_vkCmdSetDepthWriteEnableEXT(cmd, depthState->depthWriteEnable);
    g_vkCmdSetDepthCompareOpEXT(cmd, convertComparisonFunc(depthState->depthCompareOp));
}

void CommandBuffer::bindStencilState(const StencilState* stencilState) const VULKAN_NOEXCEPT
{

    if (stencilState == nullptr || mStencilState == *stencilState) {
        return;
    }

    mStencilState = *stencilState;

    g_vkCmdSetStencilTestEnableEXT(cmd, stencilState->stencilTestEnable);
}

void CommandBuffer::bindPipelineState(const PipelineState* pipelineState) const VULKAN_NOEXCEPT
{
    setPrimitiveTopology(pipelineState->primitiveState.primitiveTopology);
    setPrimitiveRestartEnable(pipelineState->primitiveState.restartEnable);

    bindShaderProgram(pipelineState->program);

    if (pipelineState->program->vertexInput) {
        setVertexInput(pipelineState->program->vertexInput);
    }

    bindRasterState(&pipelineState->rasterState);
    bindDepthState(&pipelineState->depthState);
    bindColorBlendState(&pipelineState->colorBlendState);
    bindStencilState(pipelineState->stencilState);
}

void CommandBuffer::copyImage(VulkanTexture* srcTex, VulkanTexture* destTex, const VkImageCopy* pRegions, uint32_t regionCount)
{
    /*
	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = numMips;
	subresourceRange.layerCount = 6;

	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.image = offscreen.image;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	// Copy region for transfer from framebuffer to cube face
	VkImageCopy copyRegion{};

	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset = { 0, 0, 0 };

	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.baseArrayLayer = f;
	copyRegion.dstSubresource.mipLevel = m;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset = { 0, 0, 0 };

	copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
	copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
	copyRegion.extent.depth = 1;

	vkCmdCopyImage(
		cmd,
        srcTex->image(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        destTex->image(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion);

	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.image = offscreen.image;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}*/

}

constexpr ResourceState g_UndefinedState = static_cast<ResourceState>(-1);

VkImageLayout ConvertToLayout(ResourceState state)
{
    switch (state) {
    case ResourceState::COMMON_RESOURCE:
        return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::RENDER_TARGET_RESOURCE:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ResourceState::UNORDERED_ACCESS:
        return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::DEPTH_WRITE:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ResourceState::DEPTH_READ:
    case ResourceState::DEPTH_SHADER_RESOURCE:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::NONPIXEL_SHADER_RESOURCE:
    case ResourceState::PIXEL_SHADER_RESOURCE:
    case ResourceState::SHADER_RESOURCE:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::COPY_DEST:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ResourceState::COPY_SOURCE:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::GENERICREAD:
        return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::PRESENT:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case ResourceState::SHADING_RATE_SOURCE:
        return VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
    case g_UndefinedState:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    // Unsupported states
    case ResourceState::VERTEX_BUFFER_RESOURCE:
    case ResourceState::CONSTANT_BUFFER_RESOURCE:
    case ResourceState::INDEX_BUFFER_RESOURCE:
    case ResourceState::INDIRECT_ARGUMENT:
    case ResourceState::RESOLVE_DEST:
    case ResourceState::RESOLVE_SOURCE:
    case ResourceState::RT_ACCELERATION_STRUCT:
    default:
        // CauldronCritical(L"Unsupported resource state for layout.");
        return VK_IMAGE_LAYOUT_UNDEFINED;
    };
}

VkAccessFlags ConvertToAccessMask(ResourceState state)
{
    switch (state) {
    case ResourceState::COMMON_RESOURCE:
        return 0; // VK_ACCESS_NONE_KHR;
    case ResourceState::VERTEX_BUFFER_RESOURCE:
        return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case ResourceState::CONSTANT_BUFFER_RESOURCE:
        return VK_ACCESS_UNIFORM_READ_BIT;
    case ResourceState::INDEX_BUFFER_RESOURCE:
        return VK_ACCESS_INDEX_READ_BIT;
    case ResourceState::RENDER_TARGET_RESOURCE:
        return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case ResourceState::UNORDERED_ACCESS:
        return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case ResourceState::DEPTH_WRITE:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case ResourceState::DEPTH_READ:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case ResourceState::DEPTH_SHADER_RESOURCE:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    case ResourceState::NONPIXEL_SHADER_RESOURCE:
    case ResourceState::PIXEL_SHADER_RESOURCE:
    case ResourceState::SHADER_RESOURCE:
        return VK_ACCESS_SHADER_READ_BIT;
    case ResourceState::INDIRECT_ARGUMENT:
        return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    case ResourceState::COPY_DEST:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    case ResourceState::COPY_SOURCE:
        return VK_ACCESS_TRANSFER_READ_BIT;
    case ResourceState::RESOLVE_DEST:
        break;
    case ResourceState::RESOLVE_SOURCE:
        break;
    case ResourceState::RT_ACCELERATION_STRUCT:
        return VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        // return VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    case ResourceState::SHADING_RATE_SOURCE:
        return VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
    case ResourceState::GENERICREAD:
        break;
    case ResourceState::PRESENT:
        return 0; // VK_ACCESS_NONE_KHR;
    case g_UndefinedState:
        return 0; // VK_ACCESS_NONE_KHR;
    };

    LOG_ERROR("Unsupported resource state for access mask.");
    return 0; // VK_ACCESS_NONE_KHR;
}

void SetSubResourceRange(const HwResource* pResource, VkImageMemoryBarrier& imageBarrier, uint32_t subResource)
{
    // CauldronAssert(ASSERT_CRITICAL, pResource->GetImpl()->GetResourceType() == ResourceType::Image, L"Only images support subresource.");

    const VulkanTexture* vkTexture = static_cast<const VulkanTexture*>(pResource);
    imageBarrier.subresourceRange.aspectMask = imgutil::getAspectFlags(vkTexture->vkFormat);
    if (subResource == 0xffffffff) {
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    } else {
        // VkImageCreateInfo createInfo = pResource->GetImpl()->GetImageCreateInfo();
        //  For types that have both depth and stencil, we need to correct the aspect mask and re-index the sub resource.
        if (imageBarrier.subresourceRange.aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) // depth + stencil
        {
            uint32_t numDepthSubResources = vkTexture->mipLevels * vkTexture->layerCount;
            if (subResource >= numDepthSubResources) // stencil
            {
                imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                subResource -= numDepthSubResources;
            } else // depth
            {
                imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
        }

        imageBarrier.subresourceRange.baseMipLevel = subResource % vkTexture->mipLevels;
        imageBarrier.subresourceRange.levelCount = 1;
        imageBarrier.subresourceRange.baseArrayLayer = subResource / vkTexture->mipLevels;
        imageBarrier.subresourceRange.layerCount = 1;

        // CauldronAssert(
        //     ASSERT_CRITICAL, imageBarrier.subresourceRange.baseMipLevel < createInfo.mipLevels, L"Subresource range is outside of the image range.");
        // CauldronAssert(
        //     ASSERT_CRITICAL, imageBarrier.subresourceRange.baseArrayLayer < createInfo.arrayLayers, L"Subresource range is outside of the image range.");
    }
}

void CommandBuffer::resourceBarrier(uint32_t barrierCount, const Barrier* pBarriers) const VULKAN_NOEXCEPT
{
    std::vector<VkImageMemoryBarrier> imageBarriers;
    std::vector<VkBufferMemoryBarrier> bufferBarriers;

    for (uint32_t i = 0; i < barrierCount; ++i) {
        const Barrier barrier = pBarriers[i];

        if (barrier.type == BarrierType::TRANSITION) {
            assert(barrier.sourceState == barrier.pResource->getCurrentResourceState(barrier.subResource) && "ResourceBarrier::Error : ResourceState and Barrier.SourceState do not match.");
            if (barrier.pResource->type == ResourceType::BUFFER) {
                const VulkanBuffer* vkBuffer = static_cast<const VulkanBuffer*>(barrier.pResource);
                VkBufferMemoryBarrier bufferBarrier;
                bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                bufferBarrier.pNext = nullptr;
                bufferBarrier.srcAccessMask = ConvertToAccessMask(barrier.sourceState);
                bufferBarrier.dstAccessMask = ConvertToAccessMask(barrier.destState);
                bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                bufferBarrier.buffer = vkBuffer->buffer;
                bufferBarrier.offset = 0;
                bufferBarrier.size = VK_WHOLE_SIZE;

                bufferBarriers.push_back(bufferBarrier);
            } else {
                const VulkanTexture* vkTexture = static_cast<const VulkanTexture*>(barrier.pResource);
                if ((barrier.sourceState == ResourceState::PRESENT || barrier.sourceState == g_UndefinedState)
                    && (barrier.destState == ResourceState::PIXEL_SHADER_RESOURCE
                        || barrier.destState == ResourceState::NONPIXEL_SHADER_RESOURCE
                        || barrier.destState == (ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NONPIXEL_SHADER_RESOURCE))) {
                    // Add an intermediate transition to get rid of the validation warning:
                    // we are transitioning from undefined state (which means the content of the texture is undefined) to a read state.
                    // Vulkan triggers a warning in this case.
                    // More case might exist. We will add them when we meet them.

                    VkImageMemoryBarrier imageBarrier;
                    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageBarrier.pNext = nullptr;
                    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    SetSubResourceRange(barrier.pResource, imageBarrier, barrier.subResource);
                    imageBarrier.image = vkTexture->image();

                    imageBarrier.srcAccessMask = 0;
                    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                    VkImageUsageFlags usage = vkTexture->usage;
                    if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0) {
                        imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    } else if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
                        imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    } else if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0) {
                        imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    } else if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) {
                        imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                        imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    } else {
                        LOG_WARNING("Unable to find an appropriate intermediate transition. Please support this case.");
                    }

                    // push intermediate barrier
                    imageBarriers.push_back(imageBarrier);

                    // push final barrier
                    imageBarrier.srcAccessMask = imageBarrier.dstAccessMask;
                    imageBarrier.dstAccessMask = ConvertToAccessMask(barrier.destState);
                    imageBarrier.oldLayout = imageBarrier.newLayout;
                    imageBarrier.newLayout = ConvertToLayout(barrier.destState);
                    imageBarriers.push_back(imageBarrier);
                } else {
                    VkImageMemoryBarrier imageBarrier = {};
                    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageBarrier.pNext = nullptr;
                    imageBarrier.srcAccessMask = ConvertToAccessMask(barrier.sourceState);
                    imageBarrier.dstAccessMask = ConvertToAccessMask(barrier.destState);
                    imageBarrier.oldLayout = barrier.sourceState == ResourceState::PRESENT ? VK_IMAGE_LAYOUT_UNDEFINED : ConvertToLayout(barrier.sourceState);
                    imageBarrier.newLayout = ConvertToLayout(barrier.destState);
                    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageBarrier.subresourceRange.aspectMask = imgutil::getAspectFlags(vkTexture->vkFormat);
                    SetSubResourceRange(barrier.pResource, imageBarrier, barrier.subResource);
                    imageBarrier.image = vkTexture->image();

                    imageBarriers.push_back(imageBarrier);
                }
            }

            // Set the new internal state (this is largely used for debugging
            const_cast<HwResource*>(barrier.pResource)->setCurrentResourceState(barrier.destState, barrier.subResource);
        } else if (barrier.type == BarrierType::UAV) {
            // Resource is expected to be in UAV state
            // CauldronAssert(ASSERT_CRITICAL,
            //               ResourceState::UnorderedAccess == barrier.pResource->GetCurrentResourceState(barrier.SubResource) ||
            //                   ResourceState::RTAccelerationStruct == barrier.pResource->GetCurrentResourceState(barrier.SubResource),
            //               L"ResourceBarrier::Error : ResourceState isn't UnorderedAccess or RTAccelerationStruct.");

            if (barrier.pResource->type == ResourceType::IMAGE) {
                const VulkanTexture* vkTexture = static_cast<const VulkanTexture*>(barrier.pResource);
                VkFormat imageFormat = vkTexture->vkFormat;

                VkImageMemoryBarrier imageBarrier = {};
                imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageBarrier.pNext = nullptr;
                imageBarrier.srcAccessMask = ConvertToAccessMask(barrier.sourceState); // Is this really needed for a UAV barrier? Remove if it's ignored
                imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                imageBarrier.oldLayout = barrier.sourceState == ResourceState::PRESENT ? VK_IMAGE_LAYOUT_UNDEFINED : ConvertToLayout(barrier.sourceState);
                imageBarrier.newLayout = ConvertToLayout(barrier.destState);
                imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageBarrier.subresourceRange.aspectMask = imgutil::getAspectFlags(imageFormat);
                SetSubResourceRange(barrier.pResource, imageBarrier, barrier.subResource);
                imageBarrier.image = vkTexture->image();

                imageBarriers.push_back(imageBarrier);
            } else if (barrier.pResource->type == ResourceType::BUFFER) {
                const VulkanBuffer* vkBuffer = static_cast<const VulkanBuffer*>(barrier.pResource);

                VkBufferMemoryBarrier bufferBarrier;
                bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                bufferBarrier.pNext = nullptr;
                bufferBarrier.srcAccessMask = ConvertToAccessMask(barrier.sourceState); // Is this really needed for a UAV barrier? Remove if it's ignored
                bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                bufferBarrier.buffer = vkBuffer->buffer;
                bufferBarrier.offset = 0;
                bufferBarrier.size = VK_WHOLE_SIZE;

                bufferBarriers.push_back(bufferBarrier);
            }
        } else {
            LOG_ERROR("Unsupported barrier");
        }
    }

    if (bufferBarriers.size() > 0 || imageBarriers.size() > 0) {
        uint32_t srcStageMask = VK_PIPELINE_STAGE_NONE;
        uint32_t dstStageMask = VK_PIPELINE_STAGE_NONE;
        switch (getCommandQueueType()) {
        case CommandQueueType::Graphics:
            srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
        case CommandQueueType::Compute:
            srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
        case CommandQueueType::Copy:
            srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        }
        vkCmdPipelineBarrier(
            cmd,
            srcStageMask, dstStageMask, 0,
            0, nullptr,
            static_cast<uint32_t>(bufferBarriers.size()),
            bufferBarriers.data(),
            static_cast<uint32_t>(imageBarriers.size()),
            imageBarriers.data());
    }
}

void CommandBuffer::free() const
{
    if (cmd != VK_NULL_HANDLE && commandPool != nullptr) {
        commandPool->free();
        commandPool = nullptr;
    }
}

CommandQueueType CommandBuffer::getCommandQueueType() const
{
    return commandPool->commandQueueType;
}

VkAccessFlags accessFlagsForLayout(VkImageLayout layout)
{
    switch (layout) {
    case VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED:
        return VkAccessFlagBits::VK_ACCESS_HOST_WRITE_BIT;
    case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
    case VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
    default:
        return VkAccessFlags();
    }
}

VkPipelineStageFlags pipelineStageForLayout(VkImageLayout layout)
{
    switch (layout) {
    case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;

    case VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    case VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    case VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    case VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED:
        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_HOST_BIT;

    case VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED:
        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    default:
        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
}

void CommandBuffer::setImageLayout(VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange) const VULKAN_NOEXCEPT
{
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = accessFlagsForLayout(oldImageLayout);
    imageMemoryBarrier.dstAccessMask = accessFlagsForLayout(newImageLayout);
    VkPipelineStageFlags srcStageMask = pipelineStageForLayout(oldImageLayout);
    VkPipelineStageFlags destStageMask = pipelineStageForLayout(newImageLayout);
    // Put barrier on top
    // Put barrier inside setup command buffer
    pipelineBarrier(srcStageMask, destStageMask, VkDependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void CommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers) const VULKAN_NOEXCEPT
{
    vkCmdPipelineBarrier(cmd,
        static_cast<VkPipelineStageFlags>(srcStageMask),
        static_cast<VkPipelineStageFlags>(dstStageMask),
        static_cast<VkDependencyFlags>(dependencyFlags),
        memoryBarrierCount,
        reinterpret_cast<const VkMemoryBarrier*>(pMemoryBarriers),
        bufferMemoryBarrierCount,
        reinterpret_cast<const VkBufferMemoryBarrier*>(pBufferMemoryBarriers),
        imageMemoryBarrierCount,
        reinterpret_cast<const VkImageMemoryBarrier*>(pImageMemoryBarriers));
}

void CommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    std::span<const VkMemoryBarrier> const& memoryBarriers,
    std::span<const VkBufferMemoryBarrier> const& bufferMemoryBarriers,
    std::span<const VkImageMemoryBarrier> const& imageMemoryBarriers) const VULKAN_NOEXCEPT
{
    vkCmdPipelineBarrier(cmd,
        static_cast<VkPipelineStageFlags>(srcStageMask),
        static_cast<VkPipelineStageFlags>(dstStageMask),
        static_cast<VkDependencyFlags>(dependencyFlags),
        (uint32_t)memoryBarriers.size(),
        reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.data()),
        (uint32_t)bufferMemoryBarriers.size(),
        reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers.data()),
        (uint32_t)imageMemoryBarriers.size(),
        reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers.data()));
}
}