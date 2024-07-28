#pragma once
#include "VulkanDefs.h"
#include <utility>

namespace mygfx
{

	template<typename T>
	struct VkObject2Type
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_UNKNOWN;
	};

	template<class T>
	class VkHandleBase {
	public:
		VkHandleBase() = default;
		VkHandleBase(const T& handle) : handle_(handle) {}

		inline operator T() const {
			return handle_;
		}

		inline const T& handle() const { return handle_; }

		inline explicit operator bool() const noexcept
		{
			return handle_ != VK_NULL_HANDLE;
		}

		inline bool operator!() const noexcept
		{
			return handle_ == VK_NULL_HANDLE;
		}

		void setResourceName(const char* name)
		{
			extern class VulkanDevice& gfx();

			if (name) {
				gfx().setResourceName(VkObject2Type<T>::objectType, (uint32_t)handle_, name);
			}

		}

	protected:
		T handle_ = nullptr;
	};

	template<class T>
	class HandleUnique : public VkHandleBase<T>
	{
	public:
		using VkHandleBase<T>::VkHandleBase;

		HandleUnique(const HandleUnique&& other)
		{ 
			this->handle_ = std::move(other.handle_);
		}

		HandleUnique(const HandleUnique&) = delete;
	};


	template<> struct VkObject2Type<VkInstance>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_INSTANCE;
	};

	template<> struct VkObject2Type<VkPhysicalDevice>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
	};

	template<> struct VkObject2Type<VkDevice>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_DEVICE;
	};

	template<> struct VkObject2Type<VkQueue>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_QUEUE;
	};

	template<> struct VkObject2Type<VkSemaphore>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_SEMAPHORE;
	};

	template<> struct VkObject2Type<VkCommandBuffer>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
	};

	template<> struct VkObject2Type<VkFence>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_FENCE;
	};

	template<> struct VkObject2Type<VkDeviceMemory>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
	};

	template<> struct VkObject2Type<VkBuffer>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_BUFFER;
	};

	template<> struct VkObject2Type<VkImage>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_IMAGE;
	};

	template<> struct VkObject2Type<VkEvent>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_EVENT;
	};

	template<> struct VkObject2Type<VkQueryPool>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_QUERY_POOL;
	};

	template<> struct VkObject2Type<VkBufferView>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
	};

	template<> struct VkObject2Type<VkImageView>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
	};

	template<> struct VkObject2Type<VkShaderModule>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_SHADER_MODULE;
	};

	template<> struct VkObject2Type<VkPipelineCache>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_PIPELINE_CACHE;
	};

	template<> struct VkObject2Type<VkPipelineLayout>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
	};

	template<> struct VkObject2Type<VkRenderPass>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_RENDER_PASS;
	};

	template<> struct VkObject2Type<VkPipeline>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_PIPELINE;
	};

	template<> struct VkObject2Type<VkDescriptorSetLayout>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
	};

	template<> struct VkObject2Type<VkSampler>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_SAMPLER;
	};

	template<> struct VkObject2Type<VkDescriptorPool>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
	};

	template<> struct VkObject2Type<VkDescriptorSet>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
	};

	template<> struct VkObject2Type<VkFramebuffer>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_FRAMEBUFFER;
	};

	template<> struct VkObject2Type<VkCommandPool>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_COMMAND_POOL;
	};

	template<> struct VkObject2Type<VkPrivateDataSlot>
	{
		static const VkObjectType objectType = VK_OBJECT_TYPE_PRIVATE_DATA_SLOT;
	};


}
