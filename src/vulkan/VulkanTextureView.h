#pragma once
#include "../GraphicsHandles.h"
#include "VulkanObjects.h"

namespace mygfx {
class VulkanTextureView : public HwTextureView, public HandleUnique<VkImageView> {
public:
    VulkanTextureView(const VkImageViewCreateInfo& view_info, const char* resName = 0);
    VulkanTextureView(const VkImageViewCreateInfo& view_info, SamplerHandle sampler, VkImageLayout imageLayout, const char* resName = 0);
    ~VulkanTextureView();

    inline VkImageViewType viewType() const { return viewInfo_.viewType; }
    inline const VkImageSubresourceRange& subresourceRange() const { return viewInfo_.subresourceRange; }
    inline const VkDescriptorImageInfo& descriptorInfo() const { return descriptor_; }

    void updateDescriptor(SamplerHandle sampler, VkImageLayout imageLayout);
    void updateDescriptor(VkSampler sampler, VkImageLayout imageLayout);

    void destroy();

private:
    VkImageViewCreateInfo viewInfo_;
    VkDescriptorImageInfo descriptor_;

    friend class DescriptorTable;
};
}
