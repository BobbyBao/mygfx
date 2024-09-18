#pragma once
#include "Fwd.h"
#include "utils/BitmaskEnum.h"
#include <filesystem>

namespace mygfx {

enum class FileMode : uint8_t {
    Read = (1 << 0),
    Write = (1 << 1),
    ReadWrite = Read | Write,
    Append = (1 << 2),
};

class Stream : public RefCounted {
public:
    virtual bool isOpen() const = 0;
    virtual size_t getSize() const = 0;
    virtual void close() = 0;

    virtual size_t seek(size_t offset) = 0;
    virtual size_t read(void* buffer, size_t size) = 0;
    virtual size_t write(const void* buffer, size_t size) = 0;

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

class FileSystem {
public:
    inline static Path sBasePath;
    static void setBasePath(const Path& path);
    static const Path& getCurrentPath();
    static void pushPath(const Path& path);
    static void popPath();
    static bool exist(const Path& path);
    static Path convertPath(const Path& path);
    static std::vector<uint8_t> readAll(const Path& path) noexcept;
    static std::string readAllText(const Path& path) noexcept;
};

} // namespace mygfx

template <>
struct mygfx::EnableBitMaskOperators<mygfx::FileMode>
    : public std::true_type { };
