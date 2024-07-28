#pragma once
#include "../GraphicsDefs.h"
#include "CommandBuffer.h"
#include "../FrameListener.h"
#include "VulkanObjects.h"

namespace mygfx
{
	class CommandPool : public HandleUnique<VkCommandPool> {
	public:
		CommandPool() = default;
		CommandPool(CommandPool&& other);
		CommandPool(uint32_t queueFamilyIndex);
		~CommandPool();

		void create(uint32_t queueFamilyIndex);

		VkCommandBuffer alloc(bool isSecond = false);
		void alloc(uint32_t count, VkCommandBuffer* cmds, bool isSecond = false);
		void free(const VkCommandBuffer& cmd);
		void free(uint32_t count, VkCommandBuffer* cmds);
		void reset(bool releaseResource = false);
		void destroy();
	protected:
	};

	class CommandList : CommandPool {
	public:
		using CommandPool::CommandPool;
		using CommandPool::reset;
		using CommandPool::handle;

		CommandBuffer* alloc(uint32_t count, bool isSecond = false);
		void free();

		CommandBuffer& front() { return mCommandBuffers.front(); }
		uint32_t count() const { return (uint32_t)mCommandBuffers.size(); }

		CommandQueueType commandQueueType = CommandQueueType::Graphics;
	protected:
		std::vector<CommandBuffer> mCommandBuffers;
		std::vector<VkCommandBuffer> mVkCommandBuffers;
	};
}
