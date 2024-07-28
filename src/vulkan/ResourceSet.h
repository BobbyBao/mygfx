#pragma once
#include "DescriptorSet.h"
#include <map>
#include <unordered_map>
#include <mutex>

namespace mygfx {

    class VulkanTextureView;
    
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

        template<class T>
        void setDynamicBuffer(uint32_t binding) {
            setDynamicBuffer(binding, sizeof(T));
        }

        void setDynamicBuffer(uint32_t binding, uint32_t size);
    protected:
        VkDescriptorSet defaultSet();
        void setBuffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo);
        std::map<uint32_t, DescriptorInfo> descriptorInfos_;
        Ref<DescriptorSet> defaultSet_;
        std::mutex mutex_;
        std::unordered_map<size_t, Ref<DescriptorSet>> descriptorSets_; 
    };
    
    class UniformSet : public ResourceSet {
    };

    class DescriptorTable : public ResourceSet {
    public:
        DescriptorTable(DescriptorType descriptorType);
        DescriptorTable(const DescriptorTable&) = delete;
        DescriptorTable(DescriptorTable&&) = delete;
        ~DescriptorTable();

        void init();
        
        void add(VulkanTextureView& tex, bool update = true);
        void update(VulkanTextureView& tex);
        void free(int tex);

        DescriptorSet* fragmentSet();
        VkDescriptorSet defaultSet();
    private:
        void clear();
        DescriptorType mDescriptorType = DescriptorType::CombinedImageSampler;
        std::vector<int> freeIndics_;
        Ref<DescriptorSet> fragmentSet_;
    };
    

}
