#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "VulkanDevice.h"
#include "VulkanTextureView.h"
#include "utils/Log.h"

namespace mygfx {

VkDescriptorType toVk(DescriptorType descriptorType)
{
    switch (descriptorType) {
    case DescriptorType::SAMPLER:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case DescriptorType::COMBINED_IMAGE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case DescriptorType::SAMPLED_IMAGE:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case DescriptorType::STORAGE_IMAGE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case DescriptorType::UNIFORM_TEXEL_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case DescriptorType::STORAGE_TEXEL_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case DescriptorType::UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case DescriptorType::STORAGE_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case DescriptorType::STORAGE_BUFFER_DYNAMIC:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case DescriptorType::INPUT_ATTACHMENT:
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

DescriptorSet::DescriptorSet()
{
}

DescriptorSet::DescriptorSet(const Span<DescriptorSetLayoutBinding>& bindings)
{
    init(bindings);
}

DescriptorSet::DescriptorSet(DescriptorSetLayout* layout)
{
    init(layout);
}

DescriptorSet::~DescriptorSet()
{
    destroy();
}

void DescriptorSet::init(const Span<DescriptorSetLayoutBinding>& bindings)
{
    mResourceLayout = makeShared<DescriptorSetLayout>(bindings);
    create();
}

void DescriptorSet::init(DescriptorSetLayout* layout)
{
    destroy();
    mResourceLayout = layout;
    create();
}

void DescriptorSet::create()
{
    mDescriptorResourceCounts = mResourceLayout->sizeCounts();
    mDescriptorPool = gfx().getDescriptorPools().getPool(mDescriptorResourceCounts);

    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocInfo {};

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &mResourceLayout->handle();

    if (mResourceLayout->isBindless) {
        variableDescriptorCountAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        variableDescriptorCountAllocInfo.descriptorSetCount = 1;
        variableDescriptorCountAllocInfo.pDescriptorCounts = mResourceLayout->variableDescCounts().data();
        allocInfo.pNext = &variableDescriptorCountAllocInfo;
    }

    VK_CHECK_RESULT(vkAllocateDescriptorSets(gfx().device, &allocInfo, &handle_));
}

void DescriptorSet::destroy()
{
    if (handle_) {
        auto sizeCounts = mDescriptorResourceCounts;
        auto dp = mDescriptorPool;
        auto sets = handle_;
        vkFreeDescriptorSets(gfx().device, dp, 1, &sets);
    }
}

uint32_t DescriptorSet::binding(const String& name) const
{
    auto binding = mResourceLayout->getBinding(name);
    if (binding != nullptr) {
        return binding->binding;
    }

    return -1;
}

const DescriptorSetLayoutBinding* DescriptorSet::getBinding(uint32_t index) const
{
    return mResourceLayout->getBinding(index);
}

void DescriptorSet::bind(uint32_t dstBinding, const BufferInfo& buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer.buffer);
    VkDescriptorBufferInfo info = {};
    info.buffer = vkBuffer->buffer;
    info.offset = buffer.offset;
    info.range = buffer.range;
    bind(dstBinding, info);
}

void DescriptorSet::bind(uint32_t dstBinding, HwTextureView* texView)
{
    VulkanTextureView* vkTexView = static_cast<VulkanTextureView*>(texView);
    bind(dstBinding, 0, vkTexView->descriptorInfo());
}

void DescriptorSet::bind(uint32_t dstBinding, HwTexture* tex)
{
    VulkanTexture* vkTex = static_cast<VulkanTexture*>(tex);
    bind(dstBinding, 0, vkTex->srv()->descriptorInfo());
}

void DescriptorSet::bind(uint32_t dstBinding, HwBuffer* buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    bind(dstBinding, 0, *((VkDescriptorBufferInfo*)&vkBuffer->descriptor));
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, const DescriptorInfo& descriptorInfo)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet wds {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = handle_,
        .dstBinding = dstBinding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = toVk(layoutBinding->descriptorType)
    };

    switch (descriptorInfo.index()) {
    case 0:
        wds.pImageInfo = &(VkDescriptorImageInfo&)descriptorInfo;
        break;
    case 1:
        wds.pBufferInfo = &(VkDescriptorBufferInfo&)descriptorInfo;
        break;
    case 2:
        wds.pTexelBufferView = &(VkBufferView&)descriptorInfo;
        break;
    case 3:
        auto& imageInfos = (std::vector<VkDescriptorImageInfo>&)descriptorInfo;
        wds.pImageInfo = imageInfos.data();
        wds.descriptorCount = (uint32_t)imageInfos.size();
        break;
    }

    vkUpdateDescriptorSets(gfx().device, 1, &wds, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, const Span<VkDescriptorImageInfo>& imageInfos)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = (uint32_t)imageInfos.size();
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pImageInfo = imageInfos.data();
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);

    return *this;
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, const VkDescriptorImageInfo* imageInfo, uint32_t count)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = count;
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pImageInfo = imageInfo;
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
    return *this;
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, const VkDescriptorBufferInfo* bufferInfo, uint32_t count)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = count;
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pBufferInfo = bufferInfo;
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
    return *this;
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, const VkBufferView* bufferView, uint32_t count)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = count;
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pTexelBufferView = bufferView;
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
    return *this;
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, uint32_t dstArrayElement, const VkDescriptorImageInfo& imageInfo)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = 1;
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pImageInfo = &imageInfo;
    write.dstBinding = dstBinding;
    write.dstArrayElement = dstArrayElement;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
    return *this;
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, uint32_t dstArrayElement, const VkDescriptorBufferInfo& bufferInfo)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = 1;
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pBufferInfo = &bufferInfo;
    write.dstBinding = dstBinding;
    write.dstArrayElement = dstArrayElement;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
    return *this;
}

DescriptorSet& DescriptorSet::bind(uint32_t dstBinding, uint32_t dstArrayElement, const VkBufferView& bufferView)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return *this;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = 1;
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pTexelBufferView = &bufferView;
    write.dstBinding = dstBinding;
    write.dstArrayElement = dstArrayElement;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
    return *this;
}

void DescriptorSet::bind(uint32_t dstBinding, VkImageView imageView, VkSampler pSampler, VkImageLayout imageLayout)
{
    VkDescriptorImageInfo desc_image;
    desc_image.sampler = pSampler;
    desc_image.imageView = imageView;
    desc_image.imageLayout = imageLayout;

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = 1;
    write.descriptorType = (pSampler == NULL) ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &desc_image;
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
}

void DescriptorSet::bind(uint32_t dstBinding, VkImageView imageView)
{
    VkDescriptorImageInfo desc_image;
    desc_image.sampler = VK_NULL_HANDLE;
    desc_image.imageView = imageView;
    desc_image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    write.pImageInfo = &desc_image;
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
}

void DescriptorSet::bind(uint32_t dstBinding, uint32_t descriptorsCount, const std::vector<Ref<HwTexture>>& imageViews)
{
    std::vector<VkDescriptorImageInfo> desc_images(descriptorsCount);
    uint32_t i = 0;
    for (; i < imageViews.size(); ++i) {
        // desc_images[i] = imageViews[i]->descriptorInfo();
    }
    // we should still assign the remaining descriptors
    // Using the VK_EXT_robustness2 extension, it is possible to assign a NULL one
    for (; i < descriptorsCount; ++i) {
        // desc_images[i].sampler = *Sampler::NearestClampEdge;
        desc_images[i].imageView = VK_NULL_HANDLE;
        desc_images[i].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = descriptorsCount;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = desc_images.data();
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
}

void DescriptorSet::bind(uint32_t dstBinding, uint32_t size)
{
    auto layoutBinding = mResourceLayout->getBinding(dstBinding);
    if (!layoutBinding) {
        LOG_WARNING("Cannot find DescriptorSetLayoutBinding : {}", dstBinding);
        return;
    }

    if (layoutBinding->descriptorType != DescriptorType::UNIFORM_BUFFER_DYNAMIC
        && layoutBinding->descriptorType != DescriptorType::STORAGE_BUFFER_DYNAMIC) {
        LOG_ERROR("DescriptorType must be DescriptorType::UNIFORM_BUFFER_DYNAMIC or DescriptorType::STORAGE_BUFFER_DYNAMIC.", (int)layoutBinding->descriptorType);
        return;
    }

    VkDescriptorBufferInfo out = {};
    out.buffer = gfx().getGlobalUniformBuffer();
    out.offset = 0;
    out.range = size;

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = handle_;
    write.descriptorCount = 1;
    write.descriptorType = toVk(layoutBinding->descriptorType);
    write.pBufferInfo = &out;
    write.dstArrayElement = 0;
    write.dstBinding = dstBinding;

    vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
}

}
