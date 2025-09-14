#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "utils/ThreadUtils.h"

namespace mygfx {

VulkanBuffer::VulkanBuffer(BufferUsage usage, MemoryUsage memoryUsage, uint64_t size, uint16_t stride, const void* data)
{
	this->usage = usage;
	this->size = size;
	this->stride = stride;
	this->memoryUsage = memoryUsage;

	VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if ((usage & BufferUsage::VERTEX) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	if ((usage & BufferUsage::INDEX) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}

	if ((usage & BufferUsage::UNIFORM) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	if ((usage & BufferUsage::STORAGE) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	if ((usage & BufferUsage::UNIFORM_TEXEL) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	}

	if ((usage & BufferUsage::STORAGE_TEXEL) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	}

	if ((usage & BufferUsage::INDIRECT_BUFFER) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}

	if ((usage & BufferUsage::SHADER_DEVICE_ADDRESS) != BufferUsage::NONE) {
		flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	}

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = flags;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = (VmaMemoryUsage)memoryUsage;
	allocInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
	allocInfo.pUserData = nullptr; //(void*)name;

	VmaAllocationInfo info;
	auto res = vmaCreateBuffer(gfx().getVmaAllocator(), &bufferInfo, &allocInfo, &buffer, &bufferAlloc, &info);
	assert(res == VK_SUCCESS);

	persistent = cpuVisible();

#ifdef VK_USE_PLATFORM_MACOS_MVK
	persistent = false;
#endif
	if (persistent) {
		res = vmaMapMemory(gfx().getVmaAllocator(), bufferAlloc, (void**)&mapped);
		assert(res == VK_SUCCESS);
	}

	if (data) {
		if (cpuVisible()) {
			initState(ResourceState::GENERICREAD);
		} else {
			initState(ResourceState::COPY_DEST);
		}
		setData(data, size, 0);
	}

	descriptor.buffer = buffer;
	descriptor.range = VK_WHOLE_SIZE;
	descriptor.offset = 0;

	if (flags & VkBufferUsageFlagBits::VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {

		VkBufferDeviceAddressInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		info.pNext = nullptr;
		info.buffer = buffer;

#if MYGFX_FEATURE_LEVEL <= 1
		deviceAddress = vkGetBufferDeviceAddressKHR(gfx().device, &info);
#else
		deviceAddress = vkGetBufferDeviceAddress(gfx().device, &info);
#endif
	}
}

VulkanBuffer::~VulkanBuffer()
{
	destroy();
}

bool VulkanBuffer::cpuVisible() const
{
	return memoryUsage != MemoryUsage::GPU_ONLY && memoryUsage != MemoryUsage::GPU_LAZILY_ALLOCATED;
}

void VulkanBuffer::setData(const void* data, VkDeviceSize size, VkDeviceSize offset)
{
	if (cpuVisible()) {
		void* addr = map(offset);
		memcpy(addr, data, (size_t)size);

		if (!persistent) {
			unmap();
		}

		flush(size, offset);
	} else {

		auto stage = gfx().getStagePool().acquireStage(data, size);

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;

		if (ThreadUtils::isThisThread(gfx().renderThreadID) || gfx().enalbeAsyncCopy()) {
			gfx().executeCommand(CommandQueueType::Copy, [=](auto& c) {
				vkCmdCopyBuffer(c.cmd, stage->buffer, buffer, 1, &copyRegion);
				});
		} else {
			gfx().post_async([=]() {
				gfx().executeCommand(CommandQueueType::Copy, [=](auto& c) {
					vkCmdCopyBuffer(c.cmd, stage->buffer, buffer, 1, &copyRegion);
					});
				});

		}
	}
}

void* VulkanBuffer::map(VkDeviceSize offset)
{
	if (!mapped) {
		VK_CHECK_RESULT(vmaMapMemory(gfx().getVmaAllocator(), bufferAlloc, &mapped));
	}

	return (uint8_t*)mapped + offset;
}

void VulkanBuffer::unmap()
{
	if (mapped) {
		vmaUnmapMemory(gfx().getVmaAllocator(), bufferAlloc);
		mapped = nullptr;
	}
}

void VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
	vmaFlushAllocation(gfx().getVmaAllocator(), bufferAlloc, offset, size);
}

void VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
	vmaInvalidateAllocation(gfx().getVmaAllocator(), bufferAlloc, offset, size);
}

void VulkanBuffer::copyTo(void* data, VkDeviceSize size)
{
	assert(mapped);
	memcpy(mapped, data, size);
}

void VulkanBuffer::destroy()
{
	if (buffer) {
		if (mapped) {
			vmaUnmapMemory(gfx().getVmaAllocator(), bufferAlloc);
		}

		vmaDestroyBuffer(gfx().getVmaAllocator(), buffer, bufferAlloc);
	}
}

VulkanBufferView::VulkanBufferView(VulkanBuffer* buffer, VkDeviceSize offset, VkDeviceSize range)
{
	mDescriptor.buffer = buffer->buffer;
	mDescriptor.offset = offset;
	mDescriptor.range = range;

	index_ = gfx().getStorageBufferSet()->add(mDescriptor);

}

VulkanBufferView::~VulkanBufferView()
{
	gfx().getStorageBufferSet()->free(index_);
}

}
