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
    mDSLayoutbindings.reserve(mDSLayoutbindings.size());
    mDescriptorBindingFlags.reserve(mDSLayoutbindings.size());
    mVariableDescCounts.reserve(mDSLayoutbindings.size());

    for (auto& res : bindings) {
        mDSLayoutbindings.push_back(res->dsLayoutBinding);
        mVariableDescCounts.push_back(res->dsLayoutBinding.descriptorCount);

        if (res->bindless) {
            this->isBindless = true;

            VkDescriptorBindingFlags descriptorBindingFlags = 0;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
            descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
            mDescriptorBindingFlags.push_back(descriptorBindingFlags);
        } else {
            mDescriptorBindingFlags.push_back(VkDescriptorBindingFlags {});
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
    for (uint32_t i = 0; i < mDSLayoutbindings.size(); i++) {
        if (mDSLayoutbindings[i].name == name) {
            return &mDSLayoutbindings[i];
        }
    }

    return nullptr;
}

const DescriptorSetLayoutBinding* DescriptorSetLayout::getBinding(uint32_t index) const
{
    if (index < mDSLayoutbindings.size()) {
        return &mDSLayoutbindings[index];
    }
    return nullptr;
}

DescriptorSetLayoutBinding* DescriptorSetLayout::getBinding(uint32_t index)
{
    if (index < mDSLayoutbindings.size()) {
        return &mDSLayoutbindings[index];
    }
    return nullptr;
}

ShaderStage DescriptorSetLayout::shaderStageFlags() const
{
    if (mDSLayoutbindings.size() > 0) {
        return mDSLayoutbindings[0].stageFlags;
    }
    return ShaderStage::None;
}

const VkDescriptorSetLayout& DescriptorSetLayout::handle() const
{
    if (!mHandle) {
        create();
    }

    return mHandle;
}

void DescriptorSetLayout::define(const Span<DescriptorSetLayoutBinding>& bindings, bool update)
{
    mDSLayoutbindings.assign(bindings.begin(), bindings.end());
    mDescriptorBindingFlags.resize(mDSLayoutbindings.size());
    updatable = update;
    create();
}

void DescriptorSetLayout::defineDescriptorTable(DescriptorType descriptorType, ShaderStage shaderStageFlag)
{
    isBindless = true;

    auto desciptorCount = gfx().getMaxVariableCount((VkDescriptorType)descriptorType);
    mDSLayoutbindings = { DescriptorSetLayoutBinding { 0, descriptorType, desciptorCount, shaderStageFlag } };

    VkDescriptorBindingFlags descriptorBindingFlags = 0;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    descriptorBindingFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    mDescriptorBindingFlags = { descriptorBindingFlags };
    mVariableDescCounts = { desciptorCount };

    create();
}

void DescriptorSetLayout::create() const
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (auto& binding : mDSLayoutbindings) {
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

        std::fill(mDescriptorBindingFlags.begin(), mDescriptorBindingFlags.end(), (uint32_t)descriptorBindingFlags);
        setLayoutBindingFlags.bindingCount = static_cast<uint32_t>(mDescriptorBindingFlags.size());
        setLayoutBindingFlags.pBindingFlags = mDescriptorBindingFlags.data();

        layoutInfo.pNext = &setLayoutBindingFlags;
        layoutInfo.flags = VkDescriptorSetLayoutCreateFlagBits::VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    }

    if (mHandle) {
        auto handle = mHandle;
        gfx().post([=]() {
            vkDestroyDescriptorSetLayout(gfx().device, handle, nullptr);
        });

        mDescriptorResourceCounts.fill(0);
    }

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(gfx().device, &layoutInfo, nullptr, &mHandle));

    for (auto& binding : mDSLayoutbindings) {
        mDescriptorResourceCounts[(int)binding.descriptorType] += binding.descriptorCount;
    }
}

void DescriptorSetLayout::destroy()
{
    if (mHandle) {
        auto handle = mHandle;
        gfx().post([=]() {
            vkDestroyDescriptorSetLayout(gfx().device, handle, nullptr);
        });

        mDescriptorResourceCounts.fill(0);
        mHandle = nullptr;
    }
}

size_t DescriptorSetLayout::toHash() const
{
    if (mHash == 0) {
        for (auto& binding : mDSLayoutbindings) {
            utils::hash_combine(mHash, binding.binding, binding.descriptorType, binding.descriptorCount, (uint32_t)binding.stageFlags);
        }
    }

    return mHash;
}

}