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
    Truncate = (1 << 3)
};

class IFile {
public:
public:
    IFile() = default;
    virtual ~IFile() = default;

    virtual const String& getName() const = 0;
    virtual uint64_t size() = 0;
    virtual bool isReadOnly() const = 0;
    virtual void open(FileMode mode) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;

    virtual uint64_t seek(uint64_t offset) = 0;
    virtual uint64_t tell() = 0;
    virtual uint64_t read(uint8_t* buffer, uint64_t size) = 0;
    virtual uint64_t write(const uint8_t* buffer, uint64_t size) = 0;

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
};

struct IOStream {
    bool (*exist)(const Path&) = nullptr;
    std::vector<uint8_t> (*readAll)(const Path&) = nullptr;
    std::string (*readAllText)(const Path&) = nullptr;
};

class FileSystem {
public:
    inline static IOStream sIOStream;
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

} // namespace utils

template <>
struct mygfx::EnableBitMaskOperators<mygfx::FileMode>
    : public std::true_type { };
