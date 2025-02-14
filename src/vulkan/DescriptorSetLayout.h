#pragma once
#include "../GraphicsHandles.h"
#include "../ShaderResourceInfo.h"
#include "VulkanDefs.h"
#include "utils/RefCounted.h"
#include "utils/SharedPtr.h"

namespace mygfx {

class DescriptorSetLayout : public HwObject {
public:
    DescriptorSetLayout();
    DescriptorSetLayout(const Span<DescriptorSetLayoutBinding>& bindings, bool update = false);
    DescriptorSetLayout(const std::vector<Ref<ShaderResourceInfo>>& bindings);
    ~DescriptorSetLayout();

    void define(const Span<DescriptorSetLayoutBinding>& bindings, bool update = false);
    void defineDescriptorTable(DescriptorType descriptorType, ShaderStage shaderStageFlag);

    inline uint32_t numBindings() const { return static_cast<uint32_t>(mDSLayoutbindings.size()); }

    inline const DescriptorResourceCounts& sizeCounts() const { return mDescriptorResourceCounts; }
    inline const std::vector<uint32_t>& variableDescCounts() const { return mVariableDescCounts; }

    const DescriptorSetLayoutBinding* getBinding(const String& name) const;
    const DescriptorSetLayoutBinding* getBinding(uint32_t index) const;
    DescriptorSetLayoutBinding* getBinding(uint32_t index);

    ShaderStage shaderStageFlags() const;

    const VkDescriptorSetLayout& handle() const;

    inline operator VkDescriptorSetLayout() const { return mHandle; }

    void destroy();

    bool isBindless { false };
    bool updatable = false;
    size_t toHash() const;

private:
    void create() const;
    mutable VkDescriptorSetLayout mHandle = nullptr;
    std::vector<DescriptorSetLayoutBinding> mDSLayoutbindings;
    mutable std::vector<VkDescriptorBindingFlags> mDescriptorBindingFlags;
    std::vector<uint32_t> mVariableDescCounts;
    mutable DescriptorResourceCounts mDescriptorResourceCounts = { 0 };
    mutable size_t mHash { 0 };
};

inline VkDescriptorSetLayoutBinding toVk(const DescriptorSetLayoutBinding& dsLayountBinding)
{
    return {
        .binding = dsLayountBinding.binding,
        .descriptorType = (VkDescriptorType)dsLayountBinding.descriptorType,
        .descriptorCount = dsLayountBinding.descriptorCount,
        .stageFlags = (VkShaderStageFlags)dsLayountBinding.stageFlags,
        .pImmutableSamplers = (VkSampler*)dsLayountBinding.pImmutableSamplers
    };
}
}
