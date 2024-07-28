#include "VulkanTextureView.h"
#include "ResourceSet.h"
#include "VulkanDevice.h"

namespace mygfx
{
	VulkanTextureView::VulkanTextureView(const VkImageViewCreateInfo& view_info, const char* resName)
	{
		viewInfo_ = view_info;
		vkCreateImageView(gfx().device, &view_info, nullptr, &handle_);
		
		descriptor_.imageView = handle_;
		descriptor_.sampler = VK_NULL_HANDLE;
		descriptor_.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		if (resName) {
			gfx().setResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)handle_, resName);
		}
		
	}
	
	VulkanTextureView::VulkanTextureView(const VkImageViewCreateInfo& view_info, SamplerHandle sampler, VkImageLayout imageLayout, const char* resName)
	{
		viewInfo_ = view_info;
		vkCreateImageView(gfx().device, &view_info, nullptr, &handle_);
		updateDescriptor(sampler, imageLayout);

		if (resName) {
			gfx().setResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)handle_, resName);
		}

	}

	VulkanTextureView::~VulkanTextureView()
	{
		destroy();
	}

	void VulkanTextureView::destroy()
	{
		if (handle_) {
			auto imageView = handle_;
			auto imageIndex = index_;
			
			gfx().getTextureSet()->free(imageIndex);
				vkDestroyImageView(gfx().device, imageView, nullptr);

			index_ = -1;
			handle_ = nullptr;
		}

	}
	
	void VulkanTextureView::updateDescriptor(SamplerHandle sampler, VkImageLayout imageLayout)
	{		
		descriptor_.imageView = handle_;
		descriptor_.sampler = gfx().getVkSampler(sampler);
		descriptor_.imageLayout = imageLayout;
		
		auto textureSet = gfx().getTextureSet();
		if (textureSet) {
			if (index_ == -1) {
				textureSet->add(*this);
			} else {
				textureSet->update(*this);
			}
		}


	}

	void VulkanTextureView::updateDescriptor(VkSampler sampler, VkImageLayout imageLayout)
	{
		descriptor_.imageView = handle_;
		descriptor_.sampler = sampler;
		descriptor_.imageLayout = imageLayout;
		
		auto textureSet = gfx().getTextureSet();
		if (textureSet) {
			if (index_ == -1) {
				textureSet->add(*this);
			} else {
				textureSet->update(*this);
			}
		}


	}
}