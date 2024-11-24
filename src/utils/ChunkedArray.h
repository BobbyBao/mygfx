#pragma once
#include "utils/algorithm.h"
#include <assert.h>
#include <mutex>
#include <set>
#include <vector>

namespace utils {

struct ChunkedArrayBase {
    uint32_t chunk_size = 0;
    uint32_t chunks_per_block = 0;
    uint32_t shift = 0;
    uint32_t mask = 0;
    uint32_t max_block_count;
    void** blocks = nullptr;
    std::mutex blockMutex;

    ChunkedArrayBase(uint32_t dataSize, uint32_t chunksPerBlock = 1024, uint32_t maxBlockCount = 64)
    {
        assert(utils::isPowerOfTwo(chunksPerBlock));

        chunk_size = utils::align(dataSize, 16);
        chunks_per_block = chunksPerBlock;
        shift = utils::ctz(chunksPerBlock);
        mask = chunksPerBlock - 1;
        max_block_count = maxBlockCount;
        blocks = new void*[max_block_count];
        memset(blocks, 0, sizeof(void*) * max_block_count);
        blocks[0] = createBlock();
    }

    inline uint32_t getChunkCount() const
    {
        return max_block_count * chunks_per_block;
    }

    inline uint32_t getBlockCount() const
    {
        return max_block_count;
    }

    void* createBlock()
    {
        uint32_t block_size = chunks_per_block * chunk_size;
        auto ptr = malloc(block_size);
        memset(ptr, 0, block_size);
        return ptr;
    }

    void* getAddr(uint32_t index)
    {
        assert(index < getChunkCount());

        uint32_t block = (uint32_t)index >> shift;
        uint32_t blockIndex = (uint32_t)index & mask;
        return ((char*)blocks[block]) + chunk_size * blockIndex;
    }

    void* getAddrSafe(uint32_t index)
    {
        uint32_t block = (uint32_t)index >> shift;
        uint32_t blockIndex = (uint32_t)index & mask;

        assert(block < max_block_count);

        blockMutex.lock();
        void* base = blocks[block];
        if (base == nullptr) {
            base = createBlock();
            blocks[block] = base;
        }
        blockMutex.unlock();

        return (char*)base + chunk_size * blockIndex;
    }

    void clear()
    {
        for (uint32_t i = 0; i < max_block_count; i++) {
            if (blocks[i] != nullptr)
                free(blocks[i]);
        }

        delete[] blocks;
    }
};

template <typename T>
class ChunkedArray : public ChunkedArrayBase {
public:
    ChunkedArray(uint32_t chunksPerBlock = 256, uint32_t maxBlockCount = 64)
        : ChunkedArrayBase(sizeof(T), chunksPerBlock, maxBlockCount)
    {
    }

    ~ChunkedArray()
    {
        clear();
    }

    T* getPtr(uint32_t index)
    {
        return (T*)getAddrSafe(index);
    }

    T& operator[](uint32_t index)
    {
        return *getPtr(index);
    }

    const T& operator[](uint32_t index) const
    {
        return *getPtr(index);
    }
};

template <typename T>
class ChunkedPool : public ChunkedArrayBase {
    uint32_t currentID = 0;
    std::vector<uint32_t> freeList;
    std::mutex freeListLock;

#ifndef NDEBUG
    std::set<uint32_t> activeList;
#endif
public:
    ChunkedPool(uint32_t chunksPerBlock = 256, uint32_t maxBlockCount = 64)
        : ChunkedArrayBase(sizeof(T), chunksPerBlock, maxBlockCount)
    {
    }

    ~ChunkedPool()
    {
#ifndef NDEBUG
        //
#endif
        clear();
    }

    T& operator[](uint32_t index)
    {
        return *getPtr(index);
    }

    const T& operator[](uint32_t index) const
    {
        return *getPtr(index);
    }

    T* getPtr(uint32_t index)
    {
        return (T*)getAddr(index);
    }

    T* getPtrSafe(uint32_t index)
    {
        return (T*)getAddrSafe(index);
    }

    template <typename... Args>
    uint32_t alloc(Args... args)
    {
        uint32_t index;
        {
            std::lock_guard<std::mutex> locker(freeListLock);
            if (freeList.empty()) {
                index = ++currentID;
            } else {
                index = freeList.back();
                freeList.pop_back();
            }
#ifndef NDEBUG
            activeList.insert(index);
#endif
        }

        void* addr = getPtrSafe(index);
        new (addr) T(args...);
        return index;
    }

    void free(uint32_t id)
    {
        if (id) {
            auto v = getPtr(id);
            v->~T();

            {
                std::lock_guard<std::mutex> locker(freeListLock);
                freeList.push_back(id);
#ifndef NDEBUG
                activeList.erase(id);
#endif
            }
        }
    }
};

}