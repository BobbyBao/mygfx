#include "ResourceSet.h"
#include "VulkanTexture.h"
#include "VulkanTextureView.h"
#include "VulkanDevice.h"

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
		VkDescriptorBufferInfo info = {};
		info.buffer = gfx().constBuffer();
		info.offset = 0;
		info.range = size;
		setBuffer(binding, info);
	}

	void ResourceSet::setBuffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
	{		
		descriptorInfos_[binding] = bufferInfo;

		for (auto& it : descriptorSets_) {
			it.second->bind(binding, bufferInfo);
		}

	}

	DescriptorSet* ResourceSet::getDescriptorSet(DescriptorSetLayout* layout)
	{	
		DescriptorSet*descriptorSet = nullptr;
		auto hash = layout->toHash();
		std::lock_guard locker(mutex_);
		auto it = descriptorSets_.find(hash);
		//assert(descriptorSet != nullptr);
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
		auto dsl = makeShared<DescriptorSetLayout>(bindings);
		return addDescriptorSet(dsl);
	}

	DescriptorSet* ResourceSet::addDescriptorSet(DescriptorSetLayout* layout)
	{
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

	DescriptorTable::DescriptorTable(DescriptorType descriptorType) : mDescriptorType(descriptorType)
	{
		init();
	}

	DescriptorTable::~DescriptorTable()
	{
	}

	void DescriptorTable::init()
	{
		auto descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
		descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::Fragment);
		fragmentSet_ = addDescriptorSet(descriptorSetLayoutTex);

		descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
		descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::Vertex | ShaderStage::Fragment);
		addDescriptorSet(descriptorSetLayoutTex);

		descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
		descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::Vertex | ShaderStage::TessellationEvaluation | ShaderStage::Fragment);
		addDescriptorSet(descriptorSetLayoutTex);

		descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
		descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::Vertex | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Fragment);
		addDescriptorSet(descriptorSetLayoutTex);

		descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
		descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Fragment);
		addDescriptorSet(descriptorSetLayoutTex);

		descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
		descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::Compute);
		addDescriptorSet(descriptorSetLayoutTex);

		descriptorSetLayoutTex = makeShared<DescriptorSetLayout>();
		descriptorSetLayoutTex->defineDescriptorTable(mDescriptorType, ShaderStage::Vertex | ShaderStage::Fragment | ShaderStage::Compute);
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

	void DescriptorTable::add(VulkanTextureView& tex, bool update)
	{
		std::lock_guard locker(mutex_); 
		std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);
		//assert(tex.Index < 0);
		int index;
		if (freeIndics_.size() > 0) {
			index = freeIndics_.back();
			freeIndics_.pop_back();
			imageInfos[index] = tex.descriptorInfo();
		}
		else {
			index = (int)imageInfos.size();
			imageInfos.push_back(tex.descriptorInfo());
		}

		for (auto& it : descriptorSets_) {
			it.second->bind(0, index, imageInfos[index]);
		}

		tex.index_ = index;
	}

	void DescriptorTable::update(VulkanTextureView& tex)
	{
		assert(tex.index() > 0);
		std::lock_guard locker(mutex_);
		int index = tex.index();

		std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);
		if (std::memcmp(&imageInfos[index], &tex.descriptorInfo(), sizeof(VkDescriptorImageInfo)) != 0) {
			imageInfos[index] = tex.descriptorInfo();

			for (auto& it : descriptorSets_) {
				it.second->bind(0, index, imageInfos[index]);
			}
		}

	}

	void DescriptorTable::free(int tex)
	{
		std::lock_guard locker(mutex_);
		if (tex >= 0) {
			int index = tex;

			std::vector<VkDescriptorImageInfo>& imageInfos = std::get<std::vector<VkDescriptorImageInfo>>(descriptorInfos_[0]);

			if (HwTexture::Magenta) {
				
				VulkanTexture* vkTex = static_cast<VulkanTexture*>(HwTexture::Magenta.get());
				imageInfos[index] = vkTex->srv()->descriptorInfo();
			}

			for (auto& it : descriptorSets_) {
				it.second->bind(0, index, imageInfos[index]);
			}

			freeIndics_.push_back(tex);
		}

	}

	void DescriptorTable::clear()
	{
		std::lock_guard locker(mutex_);
		descriptorInfos_.clear();
		freeIndics_.clear();
		descriptorSets_.clear();
	}


}
