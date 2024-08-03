#pragma once
#include "GraphicsDefs.h"
#include "utils/Ring.h"
#include "FrameListener.h"
#include "GraphicsHandles.h"

namespace mygfx
{
    // This class mimics the behaviour or the DX11 dynamic buffers. I can hold uniforms, index and vertex buffers.
    // It does so by suballocating memory from a huge buffer. The buffer is used in a ring fashion.  
    // Allocated memory is taken from the tail, freed memory makes the head advance;
    // See 'ring.h' to get more details on the ring buffer.
    //
    // The class knows when to free memory by just knowing:
    //    1) the amount of memory used per frame
    //    2) the number of backbuffers 
    //    3) When a new frame just started ( indicated by OnBeginFrame() )
    //         - This will free the data of the oldest frame so it can be reused for the new frame
    //
    // Note than in this ring an allocated chuck of memory has to be contiguous in memory, that is it cannot spawn accross the tail and the head.
    // This class takes care of that.

    class DynamicBufferPool : public FrameChangeListener
    {
    public:
		void create(BufferUsage usage, uint32_t numberOfBackBuffers, uint32_t memTotalSize, const char* name = NULL);
		void create(BufferUsage usage, uint32_t staticBufferSize, uint32_t numberOfBackBuffers, uint32_t memTotalSize, const char* name = NULL);
        void destroy();
        bool allocBuffer(uint32_t size, void **pData, BufferInfo*pOut);
        BufferInfo allocBuffer(uint32_t size, void *pData);
        bool allocVertexBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void **pData, BufferInfo*pOut);
        bool allocIndexBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, void **pData, BufferInfo*pOut);

        void onFrameChange() override;

        inline HwBuffer* getBuffer() const { return mBuffer; }
    private:
		uint32_t        mDynamicMemSize = 0;
		uint32_t        mStaticMemSize = 0;
        RingWithTabs    mRing;
        char           *mData = nullptr;
        SharedPtr<HwBuffer>   mBuffer;
    };
}