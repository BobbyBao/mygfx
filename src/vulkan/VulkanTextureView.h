#pragma once
#include "../GraphicsHandles.h"
#include "VulkanObjects.h"

#define USE_COMBINEDSAMPLER 0

namespace mygfx {

class VulkanSampler;

class VulkanTextureView : public HwTextureView, public HandleUnique<VkImageView> {
public:
    VulkanTextureView(const VkImageViewCreateInfo& view_info, const char* resName = 0);
    VulkanTextureView(const VkImageViewCreateInfo& view_info, VulkanSampler* sampler, VkImageLayout imageLayout, const char* resName = 0);
    ~VulkanTextureView();
    
    VkImage image() const { return mViewInfo.image; }
    VkImageViewType viewType() const { return mViewInfo.viewType; }
    VkFormat format() const { return mViewInfo.format; }
    const VkImageSubresourceRange& subresourceRange() const { return mViewInfo.subresourceRange; }
    const VkDescriptorImageInfo& descriptorInfo() const { return mDescriptor; }

    void updateDescriptor(VulkanSampler* sampler, VkImageLayout imageLayout);
    void destroy();
private:
    VkImageViewCreateInfo mViewInfo;
    VkDescriptorImageInfo mDescriptor;

    friend class DescriptorTable;
};
}
