#include "VulkanTextureView.h"
#include "ResourceSet.h"
#include "VulkanDevice.h"

namespace mygfx {
VulkanTextureView::VulkanTextureView(const VkImageViewCreateInfo& view_info, const char* resName)
{
    mViewInfo = view_info;
    vkCreateImageView(gfx().device, &view_info, nullptr, &handle_);

    mDescriptor.imageView = handle_;
    mDescriptor.sampler = VK_NULL_HANDLE;
    mDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    if (resName) {
        gfx().setResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)handle_, resName);
    }
}

VulkanTextureView::VulkanTextureView(const VkImageViewCreateInfo& view_info, VulkanSampler* sampler, VkImageLayout imageLayout, const char* resName)
{
    mViewInfo = view_info;
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
    #if USE_COMBINEDSAMPLER
    auto textureSet = gfx().getTextureSet();
    #else
    auto textureSet = gfx().getImageSet();
    #endif
        if (imageIndex != -1) {
            textureSet->free(imageIndex&0xff);
        }
        vkDestroyImageView(gfx().device, imageView, nullptr);

        index_ = -1;
        handle_ = nullptr;
    }
}

void VulkanTextureView::updateDescriptor(VulkanSampler* sampler, VkImageLayout imageLayout)
{
    mDescriptor.imageView = handle_;
    mDescriptor.sampler = sampler->vkSampler;
    mDescriptor.imageLayout = imageLayout;
    #if USE_COMBINEDSAMPLER
    auto textureSet = gfx().getTextureSet();
    #else
    auto textureSet = gfx().getImageSet();
    
    #endif

    if (index_ == -1) {
        index_ = (textureSet->add(mDescriptor) | (sampler->index << 16));
    } else {
        textureSet->update(index_&0xff, mDescriptor);
    }
    

}

}