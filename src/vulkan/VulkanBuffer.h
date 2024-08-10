#pragma once

#include "../GraphicsHandles.h"
#include "VulkanDefs.h"
#include "VulkanTools.h"
#include <vma/vk_mem_alloc.h>

namespace mygfx {

class VulkanBuffer : public HwBuffer {
public:
    VulkanBuffer(BufferUsage usage, MemoryUsage memoryUsage, uint64_t size, uint16_t stride = 0, const void* data = nullptr);
    ~VulkanBuffer();

    bool cpuVisible() const;
    void setData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation bufferAlloc = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;

    bool persistent;

    void* map(VkDeviceSize offset);
    void unmap();

    void flush(VkDeviceSize size, VkDeviceSize offset);
    void invalidate(VkDeviceSize size, VkDeviceSize offset);
    void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void copyTo(void* data, VkDeviceSize size);
    void destroy();
};

}