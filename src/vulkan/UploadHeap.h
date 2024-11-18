#pragma once

#include "CommandBuffer.h"
#include "CommandPool.h"
#include "GraphicsDevice.h"
#include <mutex>

namespace mygfx {

class Sync {
    int m_count = 0;
    std::mutex m_mutex;
    std::condition_variable condition;

public:
    int Inc()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_count++;
        return m_count;
    }

    int Dec()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_count--;
        if (m_count == 0)
            condition.notify_all();
        return m_count;
    }

    int Get()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_count;
    }

    void Reset()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_count = 0;
        condition.notify_all();
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_count != 0)
            condition.wait(lock);
    }
};

//
// This class shows the most efficient way to upload resources to the GPU memory.
// The idea is to create just one upload heap and suballocate memory from it.
// For convenience this class comes with it's own command list & submit (FlushAndFinish)
//
class UploadHeap {
public:
    void create(uint64_t uSize);
    void destroy();

    void beginBatch();
    void endBatch();

    uint8_t* Suballocate(uint64_t uSize, uint64_t uAlign);
    uint8_t* BeginSuballocate(uint64_t uSize, uint64_t uAlign);
    void EndSuballocate();
    uint8_t* BasePtr() { return m_pDataBegin; }
    VkBuffer GetResource() { return m_buffer; }

    void AddCopy(VkImage image, VkBufferImageCopy bufferImageCopy);
    void AddPreBarrier(VkImageMemoryBarrier imageMemoryBarrier);
    void AddPostBarrier(VkImageMemoryBarrier imageMemoryBarrier);

    void Flush();
    void FlushAndFinish();
    void finish();

private:
    VkBuffer m_buffer;
    VkDeviceMemory m_deviceMemory;

    uint8_t* m_pDataBegin = nullptr; // starting position of upload heap
    uint8_t* m_pDataCur = nullptr; // current position of upload heap
    uint8_t* m_pDataEnd = nullptr; // ending position of upload heap

    Sync allocating, flushing;

    struct COPY {
        VkImage m_image;
        VkBufferImageCopy m_bufferImageCopy;
    };

    std::vector<COPY> m_copies;
    std::vector<VkImageMemoryBarrier> m_toPreBarrier;
    std::vector<VkImageMemoryBarrier> m_toPostBarrier;
    std::mutex m_mutex;
    std::atomic<bool> batchUpdate = false;
};
}