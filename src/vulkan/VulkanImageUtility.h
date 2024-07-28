#pragma once
#include "../GraphicsDefs.h"
#include "VulkanDefs.h"
#include <utils/Log.h>


namespace mygfx {

	enum class VulkanLayout : uint8_t {
		// The initial layout after the creation of the VkImage. We use this to denote the state before
		// any transition.
		UNDEFINED,
		// Fragment/vertex shader accessible layout for reading and writing.
		READ_WRITE,
		// Fragment/vertex shader accessible layout for reading only.
		READ_ONLY,
		// For the source of a copy operation.
		TRANSFER_SRC,
		// For the destination of a copy operation.
		TRANSFER_DST,
		// For using a depth texture as an attachment.
		DEPTH_ATTACHMENT,
		// For using a depth texture both as an attachment and as a sampler.
		DEPTH_SAMPLER,
		// For swapchain images that will be presented.
		PRESENT,
		// For color attachments, but also used when the image is a sampler.
		// TODO: explore separate layout policies for attachment+sampling and just attachment.
		COLOR_ATTACHMENT,
		// For color attachment MSAA resolves.
		COLOR_ATTACHMENT_RESOLVE,
	};

	struct VulkanLayoutTransition {
		VkImage image;
		VulkanLayout oldLayout;
		VulkanLayout newLayout;
		VkImageSubresourceRange subresources;
	};


	namespace imgutil {
		
		VkImageAspectFlags getAspectFlags(VkFormat fmt);

		VkFormat toVk(Format fmt);

		Format fromVk(VkFormat fmt);
		
		inline VkImageViewType getViewType(SamplerType target) {
			switch (target) {
				case SamplerType::SAMPLER_CUBE:
					return VK_IMAGE_VIEW_TYPE_CUBE;
				case SamplerType::SAMPLER_1D_ARRAY:
					return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				case SamplerType::SAMPLER_2D_ARRAY:
					return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				case SamplerType::SAMPLER_CUBE_ARRAY:
					return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				case SamplerType::SAMPLER_3D:
					return VK_IMAGE_VIEW_TYPE_3D;
				case SamplerType::SAMPLER_1D:
					return VK_IMAGE_VIEW_TYPE_1D;
				default:
					return VK_IMAGE_VIEW_TYPE_2D;
			}
		}

		inline VulkanLayout getDefaultLayout(TextureUsage usage) {
			if (any(usage & TextureUsage::DepthStencilAttachment)) {
				if (any(usage & TextureUsage::Sampled)) {
					return VulkanLayout::DEPTH_SAMPLER;
				} else {
					return VulkanLayout::DEPTH_ATTACHMENT;
				}
			}

			if (any(usage & TextureUsage::ColorAttachment)) {
				return VulkanLayout::COLOR_ATTACHMENT;
			}
			// Finally, the layout for an immutable texture is optimal read-only.
			return VulkanLayout::READ_ONLY;
		}

		inline VulkanLayout getDefaultLayout(VkImageUsageFlags vkusage) {
			TextureUsage usage{};
			if (vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				usage = usage | TextureUsage::DepthStencilAttachment;
			}
			if (vkusage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
				usage = usage | TextureUsage::ColorAttachment;
			}
			if (vkusage & VK_IMAGE_USAGE_SAMPLED_BIT) {
				usage = usage | TextureUsage::Sampled;
			}
			return getDefaultLayout(usage);
		}

		constexpr inline VkImageLayout getVkLayout(VulkanLayout layout) {
			switch (layout) {
			case VulkanLayout::UNDEFINED:
				return VK_IMAGE_LAYOUT_UNDEFINED;
			case VulkanLayout::READ_WRITE:
				return VK_IMAGE_LAYOUT_GENERAL;
			case VulkanLayout::READ_ONLY:
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case VulkanLayout::TRANSFER_SRC:
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case VulkanLayout::TRANSFER_DST:
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case VulkanLayout::DEPTH_ATTACHMENT:
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case VulkanLayout::DEPTH_SAMPLER:
				return VK_IMAGE_LAYOUT_GENERAL;
			case VulkanLayout::PRESENT:
				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				// Filament sometimes samples from one miplevel while writing to another level in the
				// same texture (e.g. bloom does this). Moreover we'd like to avoid lots of expensive
				// layout transitions. So, keep it simple and use GENERAL for all color-attachable
				// textures.
			case VulkanLayout::COLOR_ATTACHMENT:
				return VK_IMAGE_LAYOUT_GENERAL;
			case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			default:
				return VK_IMAGE_LAYOUT_UNDEFINED;
			}
		}

		void transitionLayout(VkCommandBuffer cmdbuffer, VulkanLayoutTransition transition);

	} // namespace imgutil

}

bool operator<(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b);

