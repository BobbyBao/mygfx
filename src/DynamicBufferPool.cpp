#include "DynamicBufferPool.h"
#include "GraphicsDevice.h"
#include "utils/algorithm.h"

namespace mygfx
{
    void DynamicBufferPool::create(BufferUsage usage, uint32_t numberOfBackBuffers, uint32_t memTotalSize, const char *name)
    {
        mDynamicMemSize = utils::alignUp(memTotalSize, 256u);
        mRing.OnCreate(numberOfBackBuffers, mDynamicMemSize);
        mBuffer = device().createBuffer(usage, MemoryUsage::CpuToGpu, mDynamicMemSize + mStaticMemSize, 0, nullptr);
        mData = (char*)mBuffer->mapped;
    }

	void DynamicBufferPool::create(BufferUsage usage, uint32_t staticBufferSize, uint32_t numberOfBackBuffers, uint32_t memTotalSize, const char* name)
	{
		mDynamicMemSize = utils::alignUp(memTotalSize, 256u);
		mRing.OnCreate(numberOfBackBuffers, mDynamicMemSize);
		mBuffer = device().createBuffer(usage, MemoryUsage::CpuToGpu, mDynamicMemSize + mStaticMemSize, 0, nullptr);
		mData = (char*)mBuffer->mapped;
	}

    void DynamicBufferPool::destroy()
    {
        mBuffer.reset();
        mRing.destroy();
    }

    bool DynamicBufferPool::allocBuffer(uint32_t size, void **pData, BufferInfo*pOut)
    {
        size = utils::alignUp(size, 256u);

        uint32_t memOffset;
        if (mRing.Alloc(size, &memOffset) == false)
        {
            assert("Ran out of mem for 'dynamic' buffers, please increase the allocated size");
            return false;
        }

        memOffset += mStaticMemSize;

        *pData = (void *)(mData + memOffset);

        pOut->buffer = mBuffer;
        pOut->offset = memOffset;
        pOut->range = size;

        return true;
    }

    BufferInfo DynamicBufferPool::allocBuffer(uint32_t size, void *pData)
    {
        void *pBuffer;
        BufferInfo out;
        if (allocBuffer(size, &pBuffer, &out))
        {
            memcpy(pBuffer, pData, size);
        }

        return out;
    }

    bool DynamicBufferPool::allocVertexBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void **pData, BufferInfo*pOut)
    {
        return allocBuffer(numbeOfVertices * strideInBytes, pData, pOut);
    }

    bool DynamicBufferPool::allocIndexBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, void **pData, BufferInfo*pOut)
    {
        return allocBuffer(numbeOfIndices * strideInBytes, pData, pOut);
    }

    void DynamicBufferPool::onFrameChange()
    {
        mRing.OnBeginFrame();
    }


}
