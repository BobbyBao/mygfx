#include "CommandBuffer.h"
#include "VulkanImageUtility.h"
#include "CommandPool.h"
#include "VulkanDevice.h"

namespace mygfx {
	
	void CommandBuffer::begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo) const
	{
		VkCommandBufferBeginInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = 0,
			.flags = flags,
			.pInheritanceInfo = pInheritanceInfo
		};

		begin(info);
	}

	void CommandBuffer::begin(const VkCommandBufferBeginInfo& beginInfo) const
	{
		VkResult result = vkBeginCommandBuffer(cmd, reinterpret_cast<const VkCommandBufferBeginInfo*>(&beginInfo));
		VK_CHECK_MSG(result, "CommandBuffer::begin");
	}

	void CommandBuffer::end() const
	{
		VkResult result = vkEndCommandBuffer(cmd);
		VK_CHECK_MSG(result, "CommandBuffer::end");
	}

	void CommandBuffer::beginRendering(HwRenderTarget* pRT, const RenderPassInfo& renderInfo) const
	{
		VulkanRenderTarget* pVkRT = (VulkanRenderTarget*) pRT;
			
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
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0,
				1, 0, 1 });
		} else {
			//todo:check
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
					VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0,
					1, 0, 1 });
			}
		}

		bool hasStencil = true;
		if(pVkRT->depthAttachment) {
			auto& fmtInfo = getFormatInfo(pVkRT->depthAttachment->format);
			auto dstLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			if (fmtInfo.depth && !fmtInfo.stencil) {
				dstLayout =VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				hasStencil = false;
			} else if (fmtInfo.stencil && !fmtInfo.depth) {
				dstLayout =VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			}

			VkImageAspectFlags aspectMask =VK_IMAGE_ASPECT_NONE;
			if (fmtInfo.depth) {
				aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
			}

			if(fmtInfo.stencil) {
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
				VkImageSubresourceRange{ aspectMask, 0, 1, 0, 1 });
		}

		// New structures are used to define the attachments used in dynamic rendering
		VkRenderingAttachmentInfoKHR colorAttachment[8]{};
		if (pVkRT->isSwapchain) {
			colorAttachment[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			colorAttachment[0].imageView = pVkRT->colorAttachments[pVkRT->currentIndex]->rtv()->handle();
			colorAttachment[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment[0].loadOp = any(renderInfo.clearFlags & TargetBufferFlags::COLOR0) ? VK_ATTACHMENT_LOAD_OP_CLEAR 
				: any(renderInfo.loadFlags & TargetBufferFlags::COLOR0) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment[0].storeOp = any(renderInfo.storeFlags & TargetBufferFlags::COLOR0) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			std::memcpy(&colorAttachment[0].clearValue.color, renderInfo.clearColor, sizeof(float) * 4);
		} else {
			//todo:
			for (size_t i = 0; i < pVkRT->colorAttachments.size(); i++) {				
				colorAttachment[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				colorAttachment[i].imageView = pVkRT->colorAttachments[i]->rtv()->handle();
				colorAttachment[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachment[i].loadOp = any(renderInfo.clearFlags & TargetBufferFlags(1 << i)) ? VK_ATTACHMENT_LOAD_OP_CLEAR 
				: any(renderInfo.loadFlags & TargetBufferFlags(1 << i)) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment[i].storeOp = any(renderInfo.storeFlags & TargetBufferFlags(1 << i)) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
				std::memcpy(&colorAttachment[i].clearValue.color, renderInfo.clearColor, sizeof(float) * 4);
			}
		}
		// A single depth stencil attachment info can be used, but they can also be specified separately.
		// When both are specified separately, the only requirement is that the image view is identical.			
		VkRenderingAttachmentInfoKHR depthStencilAttachment{};
	
		if (pVkRT->depthAttachment) {
			auto& fmtInfo = getFormatInfo(pVkRT->depthAttachment->format);
			auto dstLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			if (fmtInfo.depth && !fmtInfo.stencil) {
				dstLayout =VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				hasStencil = false;
			} else if (fmtInfo.stencil && !fmtInfo.depth) {
				dstLayout =VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
			}
			depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			depthStencilAttachment.imageView = pVkRT->depthAttachment->dsv()->handle();
			depthStencilAttachment.imageLayout = dstLayout;
			depthStencilAttachment.loadOp = any(renderInfo.clearFlags & TargetBufferFlags::DEPTH) ? VK_ATTACHMENT_LOAD_OP_CLEAR 
				: any(renderInfo.loadFlags & TargetBufferFlags::DEPTH) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStencilAttachment.storeOp = any(renderInfo.storeFlags & TargetBufferFlags::DEPTH) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachment.clearValue.depthStencil = { InvertedDepth ? 0.0f : 1.0f,  0 };
		}

		VkRenderingInfoKHR renderingInfo{};
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

	void CommandBuffer::endRendering(HwRenderTarget* pRT) const
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
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

		} else {
			//todo:check
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
					VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
			}
		}

	}

	void CommandBuffer::resetState() const
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
		const VkColorBlendEquationEXT colorBlendEquation{};
		g_vkCmdSetColorBlendEnableEXT(cmd, 0, 1, &colorBlendEnables);
		g_vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorBlendComponentFlags);

		mProgram = nullptr;
		mVertexInput = nullptr;
		mPrimitiveState = {};
		mVertexSemantic = VertexAttribute::All;
		mRasterState = {};
		mDepthState = {};
		mColorBlendState = {};
		mStencilState = {};
		mPrimitive = nullptr;
	}
	
	void CommandBuffer::setViewportAndScissor(const VkRect2D& renderArea) const
	{
		setViewportAndScissor(renderArea.offset.x, renderArea.offset.y, renderArea.extent.width, renderArea.extent.height);
	}

	void CommandBuffer::setViewportAndScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height) const
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

	void CommandBuffer::setViewport(uint32_t viewportCount, const VkViewport* pViewports) const
	{
		g_vkCmdSetViewportWithCountEXT(cmd, viewportCount, reinterpret_cast<const VkViewport*>(pViewports));
	}

	void CommandBuffer::setScissor(uint32_t scissorCount, const VkRect2D* pScissors) const
	{
		g_vkCmdSetScissorWithCountEXT(cmd, scissorCount, reinterpret_cast<const VkRect2D*>(pScissors));
	}

	void CommandBuffer::setVertexInput(HwVertexInput* vertexInput) const {
		VulkanVertexInput* vkVertexInput = (VulkanVertexInput*)vertexInput;
		if (mVertexInput != vkVertexInput) {
			mVertexInput = vkVertexInput;

			g_vkCmdSetVertexInputEXT(cmd, (uint32_t)vkVertexInput->bindingDescriptions.size(), vkVertexInput->bindingDescriptions.data(),
				(uint32_t)vkVertexInput->attributeDescriptions.size(), vkVertexInput->attributeDescriptions.data());
		}
	}

	void CommandBuffer::setPrimitiveTopology(PrimitiveTopology primitiveTopology) const {
		if (mPrimitiveState.primitiveTopology != primitiveTopology) {
			mPrimitiveState.primitiveTopology = primitiveTopology;

			g_vkCmdSetPrimitiveTopologyEXT(cmd, (VkPrimitiveTopology)primitiveTopology);
		}
	}

	void CommandBuffer::setPrimitiveRestartEnable(bool restartEnable) const {
		g_vkCmdSetPrimitiveRestartEnableEXT(cmd, restartEnable);
	}

	void CommandBuffer::bindShaderProgram(HwProgram* program) const
	{
		VulkanProgram* vkProgram = static_cast<VulkanProgram*>(program);
		if (mProgram != program) {
			mProgram = vkProgram;
			g_vkCmdBindShadersEXT(cmd, vkProgram->stageCount, vkProgram->stages, vkProgram->shaders);
		}
	}

	void CommandBuffer::bindRasterState(const RasterState* rasterState) const {
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

	void CommandBuffer::bindColorBlendState(const ColorBlendState* colorBlendState) const {

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

        switch (func)
        {
        case CompareOp::Never:
            return VK_COMPARE_OP_NEVER;
        case CompareOp::Less:
            return invertedDepth ? VK_COMPARE_OP_GREATER : VK_COMPARE_OP_LESS;
        case CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessOrEqual:
            return invertedDepth ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:
            return invertedDepth ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterOrEqual:
            return invertedDepth ? VK_COMPARE_OP_LESS_OR_EQUAL : VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            return VK_COMPARE_OP_NEVER;
        }
    }

	void CommandBuffer::bindDepthState(const DepthState* depthState) const {
				
		if (mDepthState == *depthState) {
			return;
		}
		
		mDepthState = *depthState;

		g_vkCmdSetDepthTestEnableEXT(cmd, depthState->depthTestEnable);
		g_vkCmdSetDepthWriteEnableEXT(cmd, depthState->depthWriteEnable);
		g_vkCmdSetDepthCompareOpEXT(cmd, convertComparisonFunc(depthState->depthCompareOp));
	}
	
	void CommandBuffer::bindStencilState(const StencilState* stencilState) const {
		
		if (stencilState == nullptr || mStencilState == *stencilState) {
			return;
		}
		
		mStencilState = *stencilState;

		g_vkCmdSetStencilTestEnableEXT(cmd, stencilState->stencilTestEnable);
	}

	void CommandBuffer::bindPipelineState(const PipelineState* pipelineState) const {
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

    constexpr ResourceState g_UndefinedState = static_cast<ResourceState>(-1);

	VkImageLayout ConvertToLayout(ResourceState state)
    {
        switch (state)
        {
            case ResourceState::CommonResource:
                return VK_IMAGE_LAYOUT_GENERAL;
            case ResourceState::RenderTargetResource:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ResourceState::UnorderedAccess:
                return VK_IMAGE_LAYOUT_GENERAL;
            case ResourceState::DepthWrite:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case ResourceState::DepthRead:
            case ResourceState::DepthShaderResource:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ResourceState::NonPixelShaderResource:
            case ResourceState::PixelShaderResource:
            case ResourceState::ShaderResource:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ResourceState::CopyDest:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ResourceState::CopySource:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ResourceState::GenericRead:
                return VK_IMAGE_LAYOUT_GENERAL;
            case ResourceState::Present:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case ResourceState::ShadingRateSource:
                return VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
            case g_UndefinedState:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            // Unsupported states
            case ResourceState::VertexBufferResource:
            case ResourceState::ConstantBufferResource:
            case ResourceState::IndexBufferResource:
            case ResourceState::IndirectArgument:
            case ResourceState::ResolveDest:
            case ResourceState::ResolveSource:
            case ResourceState::RTAccelerationStruct:
            default:
                //CauldronCritical(L"Unsupported resource state for layout.");
                return VK_IMAGE_LAYOUT_UNDEFINED;
        };
    }

	VkAccessFlags ConvertToAccessMask(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::CommonResource:
            return 0; // VK_ACCESS_NONE_KHR;
        case ResourceState::VertexBufferResource:
            return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        case ResourceState::ConstantBufferResource:
            return VK_ACCESS_UNIFORM_READ_BIT;
        case ResourceState::IndexBufferResource:
            return VK_ACCESS_INDEX_READ_BIT;
        case ResourceState::RenderTargetResource:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case ResourceState::UnorderedAccess:
            return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        case ResourceState::DepthWrite:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case ResourceState::DepthRead:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        case ResourceState::DepthShaderResource:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
        case ResourceState::NonPixelShaderResource:
        case ResourceState::PixelShaderResource:
        case ResourceState::ShaderResource:
            return VK_ACCESS_SHADER_READ_BIT;
        case ResourceState::IndirectArgument:
            return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        case ResourceState::CopyDest:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case ResourceState::CopySource:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case ResourceState::ResolveDest:
            break;
        case ResourceState::ResolveSource:
            break;
        case ResourceState::RTAccelerationStruct:
            return VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
            //return VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        case ResourceState::ShadingRateSource:
            return VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
        case ResourceState::GenericRead:
            break;
        case ResourceState::Present:
            return 0; // VK_ACCESS_NONE_KHR;
        case g_UndefinedState:
            return 0;  // VK_ACCESS_NONE_KHR;
        };

        LOG_ERROR("Unsupported resource state for access mask.");
        return 0; //VK_ACCESS_NONE_KHR;
    }

	
    void SetSubResourceRange(const HwResource* pResource, VkImageMemoryBarrier& imageBarrier, uint32_t subResource)
    {
        //CauldronAssert(ASSERT_CRITICAL, pResource->GetImpl()->GetResourceType() == ResourceType::Image, L"Only images support subresource.");
       
		const VulkanTexture* vkTexture = static_cast<const VulkanTexture*>(pResource);
		imageBarrier.subresourceRange.aspectMask = imgutil::getAspectFlags(vkTexture->vkFormat);
        if (subResource == 0xffffffff)
        {
            imageBarrier.subresourceRange.baseMipLevel   = 0;
            imageBarrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
            imageBarrier.subresourceRange.baseArrayLayer = 0;
            imageBarrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
        }
        else
        {
            //VkImageCreateInfo createInfo = pResource->GetImpl()->GetImageCreateInfo();
            // For types that have both depth and stencil, we need to correct the aspect mask and re-index the sub resource.
            if (imageBarrier.subresourceRange.aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))  // depth + stencil
            {
                uint32_t numDepthSubResources = vkTexture->mipLevels * vkTexture->layerCount;
                if (subResource >= numDepthSubResources)  // stencil
                {
                    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    subResource -= numDepthSubResources;
                }
                else  // depth
                {
                    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                }
            }

            imageBarrier.subresourceRange.baseMipLevel   = subResource % vkTexture->mipLevels;
            imageBarrier.subresourceRange.levelCount     = 1;
            imageBarrier.subresourceRange.baseArrayLayer = subResource / vkTexture->mipLevels;
            imageBarrier.subresourceRange.layerCount     = 1;

            //CauldronAssert(
            //    ASSERT_CRITICAL, imageBarrier.subresourceRange.baseMipLevel < createInfo.mipLevels, L"Subresource range is outside of the image range.");
            //CauldronAssert(
            //    ASSERT_CRITICAL, imageBarrier.subresourceRange.baseArrayLayer < createInfo.arrayLayers, L"Subresource range is outside of the image range.");
        }
    }

	void CommandBuffer::resourceBarrier(uint32_t barrierCount, const Barrier* pBarriers) const
    {
        std::vector<VkImageMemoryBarrier> imageBarriers;
        std::vector<VkBufferMemoryBarrier> bufferBarriers;

        for (uint32_t i = 0; i < barrierCount; ++i)
        {
            const Barrier barrier = pBarriers[i];

            if (barrier.type == BarrierType::Transition)
            {
                assert(barrier.sourceState == barrier.pResource->getCurrentResourceState(barrier.subResource) && "ResourceBarrier::Error : ResourceState and Barrier.SourceState do not match.");
                if (barrier.pResource->type == ResourceType::Buffer)
                {
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
                }
                else
                {
					const VulkanTexture* vkTexture = static_cast<const VulkanTexture*>(barrier.pResource);
                    if ((barrier.sourceState == ResourceState::Present || barrier.sourceState == g_UndefinedState)
                        && (barrier.destState == ResourceState::PixelShaderResource
                            || barrier.destState == ResourceState::NonPixelShaderResource
                            || barrier.destState == (ResourceState::PixelShaderResource | ResourceState::NonPixelShaderResource)))
                    {
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
                        if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0)
                        {
                            imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                            imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        }
                        else if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
                        {
                            imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                            imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        }
                        else if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0)
                        {
                            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                            imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                        }
                        else if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
                        {
                            imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                            imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                        }
                        else
                        {
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
                    }
                    else
                    {
                        VkImageMemoryBarrier imageBarrier = {};
                        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                        imageBarrier.pNext = nullptr;
                        imageBarrier.srcAccessMask = ConvertToAccessMask(barrier.sourceState);
                        imageBarrier.dstAccessMask = ConvertToAccessMask(barrier.destState);
                        imageBarrier.oldLayout = barrier.sourceState == ResourceState::Present ? VK_IMAGE_LAYOUT_UNDEFINED : ConvertToLayout(barrier.sourceState);
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
            }
            else  if (barrier.type == BarrierType::UAV)
            {
                // Resource is expected to be in UAV state
                //CauldronAssert(ASSERT_CRITICAL,
                //               ResourceState::UnorderedAccess == barrier.pResource->GetCurrentResourceState(barrier.SubResource) ||
                //                   ResourceState::RTAccelerationStruct == barrier.pResource->GetCurrentResourceState(barrier.SubResource),
                //               L"ResourceBarrier::Error : ResourceState isn't UnorderedAccess or RTAccelerationStruct.");

                if (barrier.pResource->type == ResourceType::Image)
                {
					const VulkanTexture* vkTexture = static_cast<const VulkanTexture*>(barrier.pResource);
                    VkFormat imageFormat = vkTexture->vkFormat;

                    VkImageMemoryBarrier imageBarrier = {};
                    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageBarrier.pNext = nullptr;
                    imageBarrier.srcAccessMask = ConvertToAccessMask(barrier.sourceState); // Is this really needed for a UAV barrier? Remove if it's ignored
                    imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    imageBarrier.oldLayout = barrier.sourceState == ResourceState::Present ? VK_IMAGE_LAYOUT_UNDEFINED : ConvertToLayout(barrier.sourceState);
                    imageBarrier.newLayout = ConvertToLayout(barrier.destState);
                    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    imageBarrier.subresourceRange.aspectMask = imgutil::getAspectFlags(imageFormat);
                    SetSubResourceRange(barrier.pResource, imageBarrier, barrier.subResource);
                    imageBarrier.image = vkTexture->image();

                    imageBarriers.push_back(imageBarrier);
                }
                else if (barrier.pResource->type == ResourceType::Buffer)
                {
					const VulkanBuffer* vkBuffer = static_cast<const VulkanBuffer*>(barrier.pResource);

                    VkBufferMemoryBarrier bufferBarrier;
                    bufferBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    bufferBarrier.pNext               = nullptr;
                    bufferBarrier.srcAccessMask       = ConvertToAccessMask(barrier.sourceState);  // Is this really needed for a UAV barrier? Remove if it's ignored
                    bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    bufferBarrier.buffer              = vkBuffer->buffer;
                    bufferBarrier.offset              = 0;
                    bufferBarrier.size                = VK_WHOLE_SIZE;

                    bufferBarriers.push_back(bufferBarrier);
                }
            }
            else
            {
                LOG_ERROR("Unsupported barrier");
            }
        }

        if (bufferBarriers.size() > 0 || imageBarriers.size() > 0)
        {
            uint32_t srcStageMask = VK_PIPELINE_STAGE_NONE;
            uint32_t dstStageMask = VK_PIPELINE_STAGE_NONE;
            switch (getCommandQueueType())
            {
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
        if (cmd != VK_NULL_HANDLE && commandPool != nullptr)
		{
			commandPool->free();
			commandPool = nullptr;
        }
	}

	CommandQueueType CommandBuffer::getCommandQueueType() const {
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

	void CommandBuffer::setImageLayout(VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange) const VULKAN_HPP_NOEXCEPT
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
	
	void CommandBuffer::pipelineBarrier(VkPipelineStageFlags          srcStageMask,
		VkPipelineStageFlags          dstStageMask,
		VkDependencyFlags             dependencyFlags,
		uint32_t                                          memoryBarrierCount,
		const VkMemoryBarrier* pMemoryBarriers,
		uint32_t                                          bufferMemoryBarrierCount,
		const VkBufferMemoryBarrier* pBufferMemoryBarriers,
		uint32_t                                          imageMemoryBarrierCount,
		const VkImageMemoryBarrier* pImageMemoryBarriers) const VULKAN_HPP_NOEXCEPT
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

	void CommandBuffer::pipelineBarrier(VkPipelineStageFlags                            srcStageMask,
		VkPipelineStageFlags                            dstStageMask,
		VkDependencyFlags                               dependencyFlags,
		std::span<const VkMemoryBarrier> const& memoryBarriers,
		std::span<const VkBufferMemoryBarrier> const& bufferMemoryBarriers,
		std::span<const VkImageMemoryBarrier> const& imageMemoryBarriers) const VULKAN_HPP_NOEXCEPT
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