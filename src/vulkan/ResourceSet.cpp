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
    defaultSet_ = addDescriptorSet(bindings);
}

VkDescriptorSet ResourceSet::defaultSet()
{
    return *defaultSet_;
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

    descriptorInfos_[binding] = bufferInfo;

    for (auto& it : descriptorSets_) {
        it.second->bind(binding, bufferInfo);
    }
}

DescriptorSet* ResourceSet::getDescriptorSet(DescriptorSetLayout* layout)
{
    DescriptorSet* descriptorSet = nullptr;
    auto hash = layout->toHash();
    std::lock_guard locker(mutex_);
    auto it = descriptorSets_.find(hash);
    // assert(descriptorSet != nullptr);
    if (it == descriptorSets_.end()) {
        descriptorSet = new DescriptorSet(layout);
        for (auto& kvp : descriptorInfos_) {
            descriptorSet->bind(kvp.first, kvp.second);
        }
        return descriptorSets_.try_emplace(hash, descriptorSet).first->second;
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
    std::lock_guard locker(mutex_);
    auto hash = layout->toHash();
    auto it = descriptorSets_.find(hash);
    if (it == descriptorSets_.end()) {
        Ref<DescriptorSet> descriptorSet(new DescriptorSet(layout));
        for (auto& kvp : descriptorInfos_) {
            descriptorSet->bind(kvp.first, kvp.second);
        }
        return descriptorSets_.try_emplace(hash, descriptorSet).first->second;
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
    fragmentSet_ = addDescriptorSet(descriptorSetLayoutTex);

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

    descriptorInfos_[0] = std::vector<VkDescriptorImageInfo>();
}

DescriptorSet* DescriptorTable::fragmentSet()
{
    return fragmentSet_;
}

VkDescriptorSet DescriptorTable::defaultSet()
{
    return fragmentSet_->handle();
}

int DescriptorTable::add(const VkDescriptorImageInfo& descriptorInfo, bool update)
{
    std::lock_guard locker(mutex_);
    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);
    int index;
    if (freeIndics_.size() > 0) {
        index = freeIndics_.back();
        freeIndics_.pop_back();
        imageInfos[index] = descriptorInfo;
    } else {
        index = (int)imageInfos.size();
        imageInfos.push_back(descriptorInfo);
    }

    for (auto& it : descriptorSets_) {
        it.second->bind(0, index, imageInfos[index]);
    }

    return index;
}

void DescriptorTable::update(int index, const VkDescriptorImageInfo& descriptorInfo)
{
    assert(index >= 0);
    std::lock_guard locker(mutex_);

    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);
    if (std::memcmp(&imageInfos[index], &descriptorInfo, sizeof(VkDescriptorImageInfo)) != 0) {
        imageInfos[index] = descriptorInfo;

        for (auto& it : descriptorSets_) {
            it.second->bind(0, index, imageInfos[index]);
        }
    }
}

void DescriptorTable::free(int index)
{
    std::lock_guard locker(mutex_);
    if (index >= 0) {

        std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);

        if (HwTexture::Magenta) {

            VulkanTexture* vkTex = static_cast<VulkanTexture*>(HwTexture::Magenta.get());
            imageInfos[index] = vkTex->srv()->descriptorInfo();
        }

        for (auto& it : descriptorSets_) {
            it.second->bind(0, index, imageInfos[index]);
        }

        freeIndics_.push_back(index);
    }
}

void DescriptorTable::clear()
{
    std::lock_guard locker(mutex_);
    descriptorInfos_.clear();
    freeIndics_.clear();
    descriptorSets_.clear();
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

    descriptorInfos_[0] = std::vector<VkDescriptorImageInfo>();
}

Ref<SamplerHandle> SamplerTable::createSampler(SamplerInfo info)
{    
    std::lock_guard locker(mutex_);
    for (int i = 0; i < mSamplers.size(); i++) {
        if (mSamplers[i]->samplerInfo == info) {
            return mSamplers[i];
        }
    }
    
    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);

    int index = (int)imageInfos.size();
    assert(index == mSamplers.size());

    VulkanSampler* s = new VulkanSampler(info);
    mSamplers.emplace_back(s);
    
    VkDescriptorImageInfo descriptorInfo {};
    descriptorInfo.sampler = s->vkSampler;
    descriptorInfo.imageView = VK_NULL_HANDLE;
    descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfos.push_back(descriptorInfo);

    for (auto& it : descriptorSets_) {
        it.second->bind(0, index, imageInfos[index]);
    }

    s->index = index;

    return Ref<SamplerHandle>(s);
}

void SamplerTable::update(int index, const VkDescriptorImageInfo& descriptorInfo)
{
    assert(index >= 0);
    std::lock_guard locker(mutex_);

    std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);
    if (imageInfos[index] != descriptorInfo) {
        imageInfos[index] = descriptorInfo;

        for (auto& it : descriptorSets_) {
            it.second->bind(0, index, imageInfos[index]);
        }
    }
}

void SamplerTable::clear()
{
    std::lock_guard locker(mutex_);
    descriptorInfos_.clear();
    descriptorSets_.clear();
    mSamplers.clear();
}
}
