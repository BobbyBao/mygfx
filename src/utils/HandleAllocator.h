#pragma once
#include "ChunkedArray.h"

namespace utils {

template <typename T, uint32_t ChunksPerBlock, uint32_t MaxBlock>
class HandleAllocator;

template <typename T, uint32_t ChunksPerBlock = 256, uint32_t MaxBlock = 256>
struct Handle {
static constexpr const uint32_t nullid = uint32_t { 0 };
    
    constexpr Handle() noexcept = default;
    constexpr Handle(Handle const& rhs) noexcept = default;

    explicit Handle(uint32_t id, uint32_t ver = 0) noexcept
        : index(id)
        , version(ver)
    {
        assert(index != nullid);
    }
    
    uint32_t getIndex() const noexcept { return index; }
    uint32_t getVersion() const noexcept { return version; }
    bool isNull() const { return index == 0; }

    // whether this Handle is initialized
    explicit operator bool() const noexcept { return index != nullid; }

    auto operator<=>(Handle const&) const = default;

    void clear() noexcept
    {
        index = 0;
        version = 0;
    }

    T* get() const noexcept
    {
        return getAllocator().getPtr(*this);
    }

    T* operator->() const noexcept
    {
        return getAllocator().getPtr(*this);
    }

    T& operator*() const noexcept
    {
        return *get();
    }

    template <typename K>
    K* cast() const noexcept
    {
        return static_cast<K*>(get());
    }

    template <typename... Args>
    void create(Args... args)
    {
        if (!isNull()) {
            free();
        }

        *this = getAllocator().alloc(args...);
    }

    void free()
    {
        getAllocator().free(*this);
        clear();
    }
    
private:
    uint32_t index : 24 = 0;
    uint32_t version : 8 = 0;

    static HandleAllocator<T, ChunksPerBlock, MaxBlock>& getAllocator();
};

template <typename T, uint32_t ChunksPerBlock = 256, uint32_t MaxBlock = 256>
class HandleAllocator {
public:
    const uint32_t IndexBits = 24;
    const uint32_t IndexMask = 0xffffff;
    const uint32_t VersionMask = 0xff000000;

    ChunkedPool<T> manager;
    ChunkedArray<uint8_t> version;
    std::mutex versionLock;

    HandleAllocator()
        : manager(ChunksPerBlock, MaxBlock)
        , version(1024)
    {
    }

    T& operator[](uint32_t id) noexcept
    {
        return manager[id & IndexMask];
    }

    T& operator[](const Handle<T, ChunksPerBlock, MaxBlock>& id) noexcept
    {
        return manager[id.getIndex()];
    }

    T& get(const Handle<T, ChunksPerBlock, MaxBlock>& id) noexcept
    {
        return *manager.getPtr(id.getIndex());
    }

    T* getPtr(const Handle<T, ChunksPerBlock, MaxBlock>& id) noexcept
    {
        return manager.getPtr(id.getIndex());
    }

    bool isValid(const Handle<T, ChunksPerBlock, MaxBlock>& id) noexcept
    {
        return version[id.getIndex()] == id.version;
    }

    template <typename... Args>
    Handle<T, ChunksPerBlock, MaxBlock> alloc(Args... args)
    {
        auto index = manager.alloc(args...);
        versionLock.lock();
        auto ver = version[index]++;
        versionLock.unlock();
        return Handle<T, ChunksPerBlock, MaxBlock>(index, (uint32_t)ver);
    }

    void free(Handle<T, ChunksPerBlock, MaxBlock> id)
    {
        manager.free(id.getIndex());
    }
};

template <typename T, uint32_t ChunksPerBlock, uint32_t MaxBlock>
HandleAllocator<T, ChunksPerBlock, MaxBlock>& Handle<T, ChunksPerBlock, MaxBlock>::getAllocator()
{
    static HandleAllocator<T, ChunksPerBlock, MaxBlock> allocator;
    return allocator;
}

}