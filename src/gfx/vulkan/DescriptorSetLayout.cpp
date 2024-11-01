#include "DescriptorSetLayout.h"
#include "VulkanDevice.h"
#include "utils/algorithm.h"

namespace mygfx {

DescriptorSetLayout::DescriptorSetLayout()
{
}

DescriptorSetLayout::DescriptorSetLayout(const Span<DescriptorSetLayoutBinding>& bindings, bool update)
{
    define(bindings, update);
}

DescriptorSetLayout::DescriptorSetLayout(const std::vector<Ref<ShaderResourceInfo>>& bindings)
{
    dsLayoutbindings_.reserve(dsLayoutbindings_.size());
    descriptorBindingFlags_.reserve(dsLayoutbindings_.size());
    variableDescCounts_.reserve(dsLayoutbindings_.size());

    for (auto& res : bindings) {
        dsLayoutbindings_.push_back(res->dsLayoutBinding);
        variableDescCounts_.push_back(res->dsLayoutBinding.descriptorCount);

        if (res->bindless) {
            this->isBindless = true;

            VkDescriptorBindingFlags descriptorBindingFlags = 0;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
            descriptorBindingFlags_.push_back(descriptorBindingFlags);
        } else {
            descriptorBindingFlags_.push_back(VkDescriptorBindingFlags {});
        }
    }

    create();
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    destroy();
}

const DescriptorSetLayoutBinding* DescriptorSetLayout::getBinding(const String& name) const
{
    for (uint32_t i = 0; i < dsLayoutbindings_.size(); i++) {
        if (dsLayoutbindings_[i].name == name) {
            return &dsLayoutbindings_[i];
        }
    }

    return nullptr;
}

const DescriptorSetLayoutBinding& DescriptorSetLayout::getBinding(uint32_t index) const
{
    return dsLayoutbindings_[index];
}

DescriptorSetLayoutBinding& DescriptorSetLayout::getBinding(uint32_t index)
{
    return dsLayoutbindings_[index];
}

ShaderStage DescriptorSetLayout::shaderStageFlags() const
{
    if (dsLayoutbindings_.size() > 0) {
        return dsLayoutbindings_[0].stageFlags;
    }
    return ShaderStage::None;
}

const VkDescriptorSetLayout& DescriptorSetLayout::handle() const
{
    if (!handle_) {
        create();
    }

    return handle_;
}

void DescriptorSetLayout::define(const Span<DescriptorSetLayoutBinding>& bindings, bool update)
{
    dsLayoutbindings_.assign(bindings.begin(), bindings.end());
    descriptorBindingFlags_.resize(dsLayoutbindings_.size());
    updatable = update;
    create();
}

void DescriptorSetLayout::defineDescriptorTable(DescriptorType descriptorType, ShaderStage shaderStageFlag)
{
    isBindless = true;

    dsLayoutbindings_ = { DescriptorSetLayoutBinding { 0, DescriptorType::COMBINED_IMAGE_SAMPLER, VARIABLE_DESC_COUNT, shaderStageFlag } };

    VkDescriptorBindingFlags descriptorBindingFlags = 0;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    descriptorBindingFlags_ = { descriptorBindingFlags };
    variableDescCounts_ = { VARIABLE_DESC_COUNT };

    create();
}

void DescriptorSetLayout::create() const
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (auto& binding : dsLayoutbindings_) {
        bindings.push_back(toVk(binding));
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags {};
    setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    setLayoutBindingFlags.pNext = nullptr;
    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.pNext = nullptr;

    VkDescriptorBindingFlags descriptorBindingFlags = 0;
    if (isBindless) {
        descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
        descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
        descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

        std::fill(descriptorBindingFlags_.begin(), descriptorBindingFlags_.end(), (uint32_t)descriptorBindingFlags);
        setLayoutBindingFlags.bindingCount = static_cast<uint32_t>(descriptorBindingFlags_.size());
        setLayoutBindingFlags.pBindingFlags = descriptorBindingFlags_.data();

        layoutInfo.pNext = &setLayoutBindingFlags;
        layoutInfo.flags = VkDescriptorSetLayoutCreateFlagBits::VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    }

    if (handle_) {
        auto handle = handle_;
        gfx().post([=]() {
            vkDestroyDescriptorSetLayout(gfx().device, handle, nullptr);
        });

        descriptorResourceCounts_.fill(0);
    }

    vkCreateDescriptorSetLayout(gfx().device, &layoutInfo, nullptr, &handle_);

    for (auto& binding : dsLayoutbindings_) {
        descriptorResourceCounts_[(int)binding.descriptorType] += binding.descriptorCount;
    }
}

void DescriptorSetLayout::destroy()
{
    if (handle_) {
        auto handle = handle_;
        gfx().post([=]() {
            vkDestroyDescriptorSetLayout(gfx().device, handle, nullptr);
        });

        descriptorResourceCounts_.fill(0);
        handle_ = nullptr;
    }
}

size_t DescriptorSetLayout::toHash() const
{
    if (hash_ == 0) {
        for (auto& binding : dsLayoutbindings_) {
            utils::hash_combine(hash_, binding.binding, binding.descriptorType, binding.descriptorCount, (uint32_t)binding.stageFlags);
        }
    }

    return hash_;
}

}