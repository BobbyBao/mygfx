#include "CommandPool.h"
#include "VulkanDevice.h"

namespace mygfx {
CommandPool::CommandPool(CommandPool&& other)
{
    std::swap(handle_, other.handle_);
}

CommandPool::CommandPool(uint32_t queueFamilyIndex)
{
    create(queueFamilyIndex);
}

CommandPool::~CommandPool()
{
    destroy();
}

void CommandPool::create(uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.queueFamilyIndex = queueFamilyIndex;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    auto res = vkCreateCommandPool(gfx().device, &cmd_pool_info, NULL, &handle_);
    assert(res == VK_SUCCESS);
}

VkCommandBuffer CommandPool::alloc(bool isSecond)
{
    VkCommandBuffer ret;
    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = handle_;
    cmd.level = isSecond ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(gfx().device, &cmd, (VkCommandBuffer*)&ret));
    return ret;
}

void CommandPool::alloc(uint32_t count, VkCommandBuffer* cmds, bool isSecond)
{
    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = handle_;
    cmd.level = isSecond ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = count;
    VK_CHECK(vkAllocateCommandBuffers(gfx().device, &cmd, (VkCommandBuffer*)cmds));
}

void CommandPool::free(const VkCommandBuffer& cmd)
{
    vkFreeCommandBuffers(gfx().device, handle_, 1, (VkCommandBuffer*)&cmd);
}

void CommandPool::free(uint32_t count, VkCommandBuffer* cmds)
{
    vkFreeCommandBuffers(gfx().device, handle_, count, (VkCommandBuffer*)cmds);
}

void CommandPool::reset(bool releaseResource)
{
    VK_CHECK(vkResetCommandPool(gfx().device, handle_, releaseResource ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0));
}

void CommandPool::destroy()
{
    if (handle_) {
        vkDestroyCommandPool(gfx().device, handle_, nullptr);
        handle_ = nullptr;
    }
}

CommandBuffer* CommandList::alloc(uint32_t count, bool isSecond)
{
    mVkCommandBuffers.resize(count);
    mCommandBuffers.reserve(count);
    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = handle_;
    cmd.level = isSecond ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = count;
    VK_CHECK(vkAllocateCommandBuffers(gfx().device, &cmd, (VkCommandBuffer*)mVkCommandBuffers.data()));
    for (auto& c : mVkCommandBuffers) {
        auto& cb = mCommandBuffers.emplace_back(c);
        cb.commandPool = this;
    }
    return &mCommandBuffers.front();
}

void CommandList::free()
{
    if (mCommandBuffers.size() == 0) {
        return;
    }

    vkFreeCommandBuffers(gfx().device, handle_, (uint32_t)mVkCommandBuffers.size(), (VkCommandBuffer*)mVkCommandBuffers.data());
    mCommandBuffers.clear();
}

}