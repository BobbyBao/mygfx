#pragma once
#include "core/Fwd.h"

namespace mygfx {

class Stream : public RefCounted {
public:
    virtual bool isOpen() const = 0;
    virtual size_t getSize() const = 0;

    virtual size_t seek(size_t offset) = 0;
    virtual size_t read(void* buffer, size_t size) = 0;
    virtual size_t write(const void* buffer, size_t size) = 0;
    virtual void close() = 0;

    inline const String& getName() const { mName; }
    inline void setName(const String& name) { mName = name; }

    template <typename T>
    bool read(T& value)
    {
        return (read(reinterpret_cast<uint8_t*>(&value), sizeof(value)));
    }

    template <typename T>
    uint64_t write(const T& value)
    {
        return (write(reinterpret_cast<const uint8_t*>(&value), sizeof(value)));
    }

protected:
    String mName;
};

enum class FileMode : uint8_t {
    Read = (1 << 0),
    Write = (1 << 1),
    ReadWrite = Read | Write,
    Append = (1 << 2),
};


class FileStream : public Stream {
public:
    FileStream();
    FileStream(const String& fileName, FileMode mode);
    ~FileStream();

    bool open(const String& fileName, FileMode mode);

    bool isOpen() const override;
    size_t getSize() const override;
    size_t seek(size_t position) override;
    size_t read(void* dest, size_t size) override;
    size_t write(const void* data, size_t size) override;
    void flush();
    void close() override;

private:
    bool openInternal(const String& fileName, FileMode mode);
    bool readInternal(void* dest, size_t size);
    void seekInternal(size_t newPosition);

    void* mHandle = nullptr;
    FileMode mMode;
    size_t mPosition = 0;
    size_t mSize = 0;
    bool mReadSyncNeeded;
    bool mWriteSyncNeeded;

};

}

template <>
struct mygfx::EnableBitMaskOperators<mygfx::FileMode>
    : public std::true_type { };
