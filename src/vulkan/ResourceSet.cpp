#include "ResourceSet.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "VulkanTextureView.h"

namespace mygfx {

ResourceSet::ResourceSet()
{
}

ResourceSet::ResourceSet(const Span<DescriptorSetLayoutBinding>& bindings)
{
    addDescriptorSet(bindings);
}

void ResourceSet::setDynamicBuffer(uint32_t binding, uint32_t size)
{
    CHECK_RENDER_THREAD();

    VkDescriptorBufferInfo info = {};
    info.buffer = gfx().getGlobalUniformBuffer();
    info.offset = 0;
    info.range = size;
    setBuffer(binding, info);
}

void ResourceSet::setBuffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
{
    CHECK_RENDER_THREAD();

    mDescriptorInfos[binding] = bufferInfo;

    for (auto& it : mDescriptorSets) {
        it.second->bind(binding, bufferInfo);
    }
}

DescriptorSet* ResourceSet::getDescriptorSet(DescriptorSetLayout* layout)
{
    DescriptorSet* descriptorSet = nullptr;
    auto hash = layout->toHash();
    std::lock_guard locker(mMutex);
    auto it = mDescriptorSets.find(hash);
    // assert(descriptorSet != nullptr);
    if (it == mDescriptorSets.end()) {
        descriptorSet = new DescriptorSet(layout);
        for (auto& kvp : mDescriptorInfos) {
            descriptorSet->bind(kvp.first, kvp.second);
        }
        return mDescriptorSets.try_emplace(hash, descriptorSet).first->second;
    } else {
        descriptorSet = it->second;
    }

    return descriptorSet;
}

DescriptorSet* ResourceSet::addDescriptorSet(const Span<DescriptorSetLayoutBinding>& bindings)
{
    CHECK_RENDER_THREAD();

    auto dsl = makeShared<DescriptorSetLayout>(bindings);
    return addDescriptorSet(dsl);
}

DescriptorSet* ResourceSet::addDescriptorSet(DescriptorSetLayout* layout)
{    
    std::lock_guard locker(mMutex);
    auto hash = layout->toHash();
    auto it = mDescriptorSets.find(hash);
    if (it == mDescriptorSets.end()) {
        Ref<DescriptorSet> descriptorSet(new DescriptorSet(layout));
        for (auto& kvp : mDescriptorInfos) {
            descriptorSet->bind(kvp.first, kvp.second);
        }
        return mDescriptorSets.try_emplace(hash, descriptorSet).first->second;
    }

    return it->second.get();
}

DescriptorTable::DescriptorTable(DescriptorType descriptorType)
    : mDescriptorType(descriptorType)
{
    init();
}

DescriptorTable::~DescriptorTable()
{
}

void DescriptorTable::init()
{
    auto descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::FRAGMENT);
    mFragmentSet = addDescriptorSet(descriptorSetLayoutTex);

    descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::VERTEX | ShaderStage::FRAGMENT);
    addDescriptorSet(descriptorSetLayoutTex);
#if false
    descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::VERTEX | ShaderStage::TESSELLATION_EVALUATION | ShaderStage::FRAGMENT);
    addDescriptorSet(descriptorSetLayoutTex);

    descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::VERTEX | ShaderStage::TESSELLATION_CONTROL | ShaderStage::TESSELLATION_EVALUATION | ShaderStage::FRAGMENT);
    addDescriptorSet(descriptorSetLayoutTex);

    descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::TESSELLATION_CONTROL | ShaderStage::TESSELLATION_EVALUATION | ShaderStage::FRAGMENT);
    addDescriptorSet(descriptorSetLayoutTex);
#endif
    descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::COMPUTE);
    addDescriptorSet(descriptorSetLayoutTex);

    descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::VERTEX | ShaderStage::FRAGMENT | ShaderStage::COMPUTE);
    addDescriptorSet(descriptorSetLayoutTex);

    mDescriptorInfos[0] = std::vector<VkDescriptorImageInfo>();
}

DescriptorSet* DescriptorTable::fragmentSet()
{
    return mFragmentSet;
}

int DescriptorTable::add(const VkDescriptorImageInfo& descriptorInfo, bool update)
{
    std::lock_guard locker(mMutex);
    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(mDescriptorInfos[0]);
    int index;
    if (mFreeIndics.size() > 0) {
        index = mFreeIndics.back();
        mFreeIndics.pop_back();
        imageInfos[index] = descriptorInfo;
    } else {
        index = (int)imageInfos.size();
        imageInfos.push_back(descriptorInfo);
    }

    for (auto& it : mDescriptorSets) {
        it.second->bind(0, index, imageInfos[index]);
    }

    return index;
}

void DescriptorTable::update(int index, const VkDescriptorImageInfo& descriptorInfo)
{
    assert(index >= 0);
    std::lock_guard locker(mMutex);

    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(mDescriptorInfos[0]);
    if (std::memcmp(&imageInfos[index], &descriptorInfo, sizeof(VkDescriptorImageInfo)) != 0) {
        imageInfos[index] = descriptorInfo;

        for (auto& it : mDescriptorSets) {
            it.second->bind(0, index, imageInfos[index]);
        }
    }
}

void DescriptorTable::free(int index)
{
    std::lock_guard locker(mMutex);
    if (index >= 0) {

        std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(mDescriptorInfos[0]);

        if (HwTexture::Magenta) {

            VulkanTexture* vkTex = static_cast<VulkanTexture*>(HwTexture::Magenta.get());
            imageInfos[index] = vkTex->srv()->descriptorInfo();
        }

        for (auto& it : mDescriptorSets) {
            it.second->bind(0, index, imageInfos[index]);
        }

        mFreeIndics.push_back(index);
    }
}

void DescriptorTable::clear()
{
    std::lock_guard locker(mMutex);
    mDescriptorInfos.clear();
    mFreeIndics.clear();
    mDescriptorSets.clear();
}

SamplerTable::SamplerTable()
{
    init();
}

SamplerTable::~SamplerTable()
{
}

void SamplerTable::init()
{
    auto descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::FRAGMENT);
    mFragmentSet = addDescriptorSet(descriptorSetLayoutTex);

    descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
    descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::VERTEX | ShaderStage::FRAGMENT);
    addDescriptorSet(descriptorSetLayoutTex);

    mDescriptorInfos[0] = std::vector<VkDescriptorImageInfo>();
}

Ref<SamplerHandle> SamplerTable::createSampler(SamplerInfo info)
{    
    std::lock_guard locker(mMutex);
    for (int i = 0; i < mSamplers.size(); i++) {
        if (mSamplers[i]->samplerInfo == info) {
            return mSamplers[i];
        }
    }
    
    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(mDescriptorInfos[0]);

    int index = (int)imageInfos.size();
    assert(index == mSamplers.size());

    VulkanSampler* s = new VulkanSampler(info);
    mSamplers.emplace_back(s);
    
    VkDescriptorImageInfo descriptorInfo {};
    descriptorInfo.sampler = s->vkSampler;
    descriptorInfo.imageView = VK_NULL_HANDLE;
    descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfos.push_back(descriptorInfo);

    for (auto& it : mDescriptorSets) {
        it.second->bind(0, index, imageInfos[index]);
    }

    s->index = index;

    return Ref<SamplerHandle>(s);
}

void SamplerTable::update(int index, const VkDescriptorImageInfo& descriptorInfo)
{
    assert(index >= 0);
    std::lock_guard locker(mMutex);

    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(mDescriptorInfos[0]);
    if (imageInfos[index] != descriptorInfo) {
        imageInfos[index] = descriptorInfo;

        for (auto& it : mDescriptorSets) {
            it.second->bind(0, index, imageInfos[index]);
        }
    }
}

void SamplerTable::clear()
{
    std::lock_guard locker(mMutex);
    mDescriptorInfos.clear();
    mDescriptorSets.clear();
    mSamplers.clear();
}
}
