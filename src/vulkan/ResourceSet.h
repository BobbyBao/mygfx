#pragma once
#include "DescriptorSet.h"
#include <map>
#include <mutex>
#include <unordered_map>

namespace mygfx {

class VulkanTextureView;
class VulkanSampler;
class DescriptorSetLayout;

class IResourceSet : public RefCounted {
public:
    virtual DescriptorSet* getDescriptorSet(DescriptorSetLayout* layout) = 0;
};

class ResourceSet : public IResourceSet {
public:
    ResourceSet();
    ResourceSet(const Span<DescriptorSetLayoutBinding>& bindings);

    DescriptorSet* getDescriptorSet(DescriptorSetLayout* layout) override;
    DescriptorSet* addDescriptorSet(const Span<DescriptorSetLayoutBinding>& bindings);
    DescriptorSet* addDescriptorSet(DescriptorSetLayout* layout);

    template <class T>
    void setDynamicBuffer(uint32_t binding)
    {
        setDynamicBuffer(binding, sizeof(T));
    }

    void setDynamicBuffer(uint32_t binding, uint32_t size);

protected:
    void setBuffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo);
    std::map<uint32_t, DescriptorInfo> mDescriptorInfos;
    std::mutex mMutex;
    std::unordered_map<size_t, Ref<DescriptorSet>> mDescriptorSets;
};

class DescriptorTable : public ResourceSet {
public:
    DescriptorTable(DescriptorType descriptorType);
    ~DescriptorTable();

    void init();

    int add(const VkDescriptorBufferInfo& descriptorInfo, bool update = true);
    void update(int index, const VkDescriptorBufferInfo& descriptorInfo);
    int add(const VkDescriptorImageInfo& descriptorInfo, bool update = true);
    void update(int index, const VkDescriptorImageInfo& descriptorInfo);
    void free(int index);
private:
    void clear();
    DescriptorSet* fragmentSet();
    DescriptorType mDescriptorType = DescriptorType::COMBINED_IMAGE_SAMPLER;
    std::vector<int> mFreeIndics;
    Ref<DescriptorSet> mFragmentSet;
};

class SamplerTable : public ResourceSet {
public:
    SamplerTable();
    ~SamplerTable();

    void init();
    
    Ref<SamplerHandle> createSampler(SamplerInfo info);

private:
    void update(int index, const VkDescriptorImageInfo& descriptorInfo);
    void clear();

    DescriptorType mDescriptorType = DescriptorType::SAMPLER;
    Ref<DescriptorSet> mFragmentSet;
    Vector<Ref<VulkanSampler>> mSamplers;
};

}
