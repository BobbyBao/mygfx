#include "UploadHeap.h"
#include "VulkanDevice.h"
#include "utils/algorithm.h"

namespace mygfx {

void UploadHeap::create(uint64_t uSize)
{
    VkResult res;

    // Create buffer to suballocate
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = uSize;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        res = vkCreateBuffer(gfx().device, &buffer_info, NULL, &m_buffer);
        assert(res == VK_SUCCESS);

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(gfx().device, m_buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_reqs.size;
        alloc_info.memoryTypeIndex = gfx().getMemoryType(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        res = vkAllocateMemory(gfx().device, &alloc_info, NULL, &m_deviceMemory);
        assert(res == VK_SUCCESS);

        res = vkBindBufferMemory(gfx().device, m_buffer, m_deviceMemory, 0);
        assert(res == VK_SUCCESS);

        res = vkMapMemory(gfx().device, m_deviceMemory, 0, mem_reqs.size, 0, (void**)&m_pDataBegin);
        assert(res == VK_SUCCESS);

        m_pDataCur = m_pDataBegin;
        m_pDataEnd = m_pDataBegin + mem_reqs.size;
    }
}

//--------------------------------------------------------------------------------------
//
// OnDestroy
//
//--------------------------------------------------------------------------------------
void UploadHeap::destroy()
{
    vkDestroyBuffer(gfx().device, m_buffer, NULL);
    vkUnmapMemory(gfx().device, m_deviceMemory);
    vkFreeMemory(gfx().device, m_deviceMemory, NULL);
}

void UploadHeap::beginBatch()
{
    batchUpdate = true;
}

void UploadHeap::endBatch()
{
    FlushAndFinish();

    batchUpdate = false;
}

void UploadHeap::finish()
{
    if (!batchUpdate) {
        FlushAndFinish();
    }
}
//--------------------------------------------------------------------------------------
//
// SuballocateFromUploadHeap
//
//--------------------------------------------------------------------------------------
uint8_t* UploadHeap::Suballocate(uint64_t uSize, uint64_t uAlign)
{
    // wait until we are done flusing the heap
    flushing.Wait();

    uint8_t* pRet = NULL;

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // make sure resource (and its mips) would fit the upload heap, if not please make the upload heap bigger
        assert(uSize < (uint64_t)(m_pDataEnd - m_pDataBegin));

        m_pDataCur = reinterpret_cast<uint8_t*>(utils::alignUp(reinterpret_cast<uint64_t>(m_pDataCur), uAlign));
        uSize = utils::alignUp(uSize, uAlign);

        // return NULL if we ran out of space in the heap
        if ((m_pDataCur >= m_pDataEnd) || (m_pDataCur + uSize >= m_pDataEnd)) {
            return NULL;
        }

        pRet = m_pDataCur;
        m_pDataCur += uSize;
    }

    return pRet;
}

uint8_t* UploadHeap::BeginSuballocate(uint64_t uSize, uint64_t uAlign)
{
    uint8_t* pRes = NULL;

    for (;;) {
        pRes = Suballocate(uSize, uAlign);
        if (pRes != NULL) {
            break;
        }

        FlushAndFinish();
    }

    allocating.Inc();

    return pRes;
}

void UploadHeap::EndSuballocate()
{
    allocating.Dec();
}

void UploadHeap::AddCopy(VkImage image, VkBufferImageCopy bufferImageCopy)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_copies.push_back({ image, bufferImageCopy });
}

void UploadHeap::AddPreBarrier(VkImageMemoryBarrier imageMemoryBarrier)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_toPreBarrier.push_back(imageMemoryBarrier);
}

void UploadHeap::AddPostBarrier(VkImageMemoryBarrier imageMemoryBarrier)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_toPostBarrier.push_back(imageMemoryBarrier);
}

void UploadHeap::Flush()
{
    VkResult res;

    VkMappedMemoryRange range[1] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = m_deviceMemory;
    range[0].size = m_pDataCur - m_pDataBegin;
    res = vkFlushMappedMemoryRanges(gfx().device, 1, range);
    assert(res == VK_SUCCESS);
}

//--------------------------------------------------------------------------------------
//
// FlushAndFinish
//
//--------------------------------------------------------------------------------------
void UploadHeap::FlushAndFinish()
{
    // make sure another thread is not already flushing
    flushing.Wait();

    // begins a critical section, and make sure no allocations happen while a thread is inside it
    flushing.Inc();

    // wait for pending allocations to finish
    allocating.Wait();

    std::unique_lock<std::mutex> lock(m_mutex);

    Flush();

    // LOG_INFO("flushing {}", m_copies.size());

    gfx().executeCommand(CommandQueueType::Copy, [this](const CommandBuffer& cmd) {
        // apply pre barriers in one go
        if (m_toPreBarrier.size() > 0) {
            vkCmdPipelineBarrier(cmd.cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, (uint32_t)m_toPreBarrier.size(), m_toPreBarrier.data());
            m_toPreBarrier.clear();
        }

        for (COPY c : m_copies) {
            vkCmdCopyBufferToImage(cmd.cmd, GetResource(), c.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &c.m_bufferImageCopy);
        }

        m_copies.clear();

        // apply post barriers in one go
        if (m_toPostBarrier.size() > 0) {
            vkCmdPipelineBarrier(cmd.cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, (uint32_t)m_toPostBarrier.size(), m_toPostBarrier.data());
            m_toPostBarrier.clear();
        }
    });

    m_pDataCur = m_pDataBegin;

    flushing.Dec();
}
}