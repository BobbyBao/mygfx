#pragma once
#include "DescriptorSetLayout.h"
#include "GraphicsHandles.h"
#include "VulkanObjects.h"
#include <variant>

namespace mygfx {

class DescriptorSet;

using DescriptorInfo = std::variant<VkDescriptorImageInfo, VkDescriptorBufferInfo, VkBufferView, std::vector<VkDescriptorImageInfo>>;

class DescriptorSet : public HwDescriptorSet, public VkHandleBase<VkDescriptorSet> {
public:
    DescriptorSet();
    DescriptorSet(const Span<DescriptorSetLayoutBinding>& bindings);
    DescriptorSet(DescriptorSetLayout* layout);
    ~DescriptorSet();

    void init(const Span<DescriptorSetLayoutBinding>& bindings);
    void init(DescriptorSetLayout* layout);

    void bind(uint32_t dstBinding, const BufferInfo& buffer);
    void bind(uint32_t dstBinding, HwTextureView* texView);
    void bind(uint32_t dstBinding, HwBuffer* buffer);

    void bind(uint32_t dstBinding, HwTexture* tex);

    DescriptorSet& bind(uint32_t dstBinding, const DescriptorInfo& descriptorInfo);

    DescriptorSet& bind(uint32_t dstBinding, const VkDescriptorImageInfo* imageInfo, uint32_t count);
    DescriptorSet& bind(uint32_t dstBinding, const VkDescriptorBufferInfo* bufferInfo, uint32_t count);
    DescriptorSet& bind(uint32_t dstBinding, const VkBufferView* bufferView, uint32_t count);

    DescriptorSet& bind(uint32_t dstBinding, uint32_t dstArrayElement, const VkDescriptorImageInfo& imageInfo);
    DescriptorSet& bind(uint32_t dstBinding, uint32_t dstArrayElement, const VkDescriptorBufferInfo& bufferInfo);
    DescriptorSet& bind(uint32_t dstBinding, uint32_t dstArrayElement, const VkBufferView& bufferView);

    void bind(uint32_t dstBinding, VkImageView imageView, VkSampler pSampler, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    void bind(uint32_t dstBinding, VkImageView imageView);
    void bind(uint32_t dstBinding, uint32_t descriptorsCount, const std::vector<Ref<HwTexture>>& imageViews);
    void bind(uint32_t dstBinding, uint32_t size);

    const VkDescriptorSetLayout& layout() const { return mResourceLayout->handle(); }
    uint32_t binding(const String& name) const;
    const DescriptorSetLayoutBinding* getBinding(uint32_t index) const;

    void destroy();

    uint32_t dynamicBufferSize[8] = { 0 };

private:
    DescriptorSet& bind(uint32_t dstBinding, const Span<VkDescriptorImageInfo>& imageInfos);
    void create();

    VkDescriptorPool mDescriptorPool;
    DescriptorResourceCounts mDescriptorResourceCounts = { 0 };
    Ref<DescriptorSetLayout> mResourceLayout;
};

}
