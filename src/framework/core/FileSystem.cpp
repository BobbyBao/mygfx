
#include "FileSystem.h"
#include "utils/Log.h"

namespace mygfx {

static const char* getOpenMode(FileMode mode)
{
    switch (mode) {
    case FileMode::Read:
        return "rb";
    case FileMode::Write:
        return "wb";
    case FileMode::Read | FileMode::Write:
        return "r+b";
    case FileMode::Append:
        return "a";
    default:
        return "rb";
    }
}

std::vector<Path> sPathStack;

class File : public Stream {

    void* mHandle = nullptr;
    FileMode mMode;
    size_t mPosition = 0;
    size_t mSize = 0;
    bool mReadSyncNeeded;
    bool mWriteSyncNeeded;

public:
    File()
        : mMode(FileMode::Read)
        , mHandle(0)
        , mReadSyncNeeded(false)
        , mWriteSyncNeeded(false)
    {
    }

    File(const String& fileName, FileMode mode)
        : mMode(FileMode::Read)
        , mHandle(0)
        , mReadSyncNeeded(false)
        , mWriteSyncNeeded(false)
    {
        open(fileName, mode);
    }

    ~File()
    {
        close();
    }

    size_t getSize() const override
    {
        return mSize;
    }

    bool open(const String& fileName, FileMode mode)
    {
        return openInternal(fileName, mode);
    }

    size_t read(void* dest, size_t size) override
    {
        if (!isOpen()) {
            // If file not open, do not log the error further here to prevent spamming
            // the stderr stream
            return 0;
        }

        if (mMode == FileMode::Write) {
            LOG_ERROR("File not opened for reading");
            return 0;
        }

        if (size + mPosition > mSize)
            size = mSize - mPosition;
        if (!size)
            return 0;

        // Need to reassign the position due to internal buffering when transitioning
        // from writing to reading
        if (mReadSyncNeeded) {
            seekInternal(mPosition);
            mReadSyncNeeded = false;
        }

        if (!readInternal(dest, size)) {
            // Return to the position where the read began
            seekInternal(mPosition);
            LOG_ERROR("Error while reading from file " + mName);
            return 0;
        }

        mWriteSyncNeeded = true;
        mPosition += size;
        return size;
    }

    size_t seek(size_t position) override
    {
        if (!isOpen()) {
            // If file not open, do not log the error further here to prevent spamming
            // the stderr stream
            return 0;
        }

        // Allow sparse seeks if writing
        if (mMode == FileMode::Read && position > mSize)
            position = mSize;

        seekInternal(position);
        mPosition = position;
        mReadSyncNeeded = false;
        mWriteSyncNeeded = false;
        return mPosition;
    }

    size_t write(const void* data, size_t size) override
    {
        if (!isOpen()) {
            // If file not open, do not log the error further here to prevent spamming
            // the stderr stream
            return 0;
        }

        if (mMode == FileMode::Read) {
            LOG_ERROR("File not opened for writing");
            return 0;
        }

        if (!size)
            return 0;

        // Need to reassign the position due to internal buffering when transitioning
        // from reading to writing
        if (mWriteSyncNeeded) {
            seekInternal(mPosition);
            mWriteSyncNeeded = false;
        }

        if (fwrite(data, size, 1, (FILE*)mHandle) != 1) {
            // Return to the position where the write began
            seekInternal(mPosition);
            LOG_ERROR("Error while writing to file " + mName);
            return 0;
        }

        mReadSyncNeeded = true;
        mPosition += size;
        if (mPosition > mSize)
            mSize = mPosition;

        return size;
    }

    void close() override
    {
        if (mHandle) {
            fclose((FILE*)mHandle);
            mHandle = 0;
            mPosition = 0;
            mSize = 0;
        }
    }

    void flush()
    {
        if (mHandle) {
            fflush((FILE*)mHandle);
        }
    }

    bool isOpen() const override
    {
        return mHandle != nullptr;
    }

    bool openInternal(const String& fileName, FileMode mode)
    {
        close();

        mReadSyncNeeded = false;
        mWriteSyncNeeded = false;

        if (fileName.empty()) {
            LOG_ERROR("Could not open file with empty name");
            return false;
        }

        mHandle = fopen(fileName.c_str(), getOpenMode(mode));

        if (!mHandle) {
            LOG_ERROR("Could not open file {}", fileName.c_str());
            return false;
        }

        fseek((FILE*)mHandle, 0, SEEK_END);
        long size = ftell((FILE*)mHandle);
        fseek((FILE*)mHandle, 0, SEEK_SET);

        if (size > UINT_MAX) {
            LOG_ERROR("Could not open file {} which is larger than 4GB",
                fileName.c_str());
            close();
            mSize = 0;
            return false;
        }

        mSize = (unsigned)size;
        mName = fileName;
        mMode = mode;
        mPosition = 0;

        return true;
    }

    bool readInternal(void* dest, size_t size)
    {
        return fread(dest, size, 1, (FILE*)mHandle) == 1;
    }

    void seekInternal(size_t newPosition)
    {
        fseek((FILE*)mHandle, (long)newPosition, SEEK_SET);
    }
};

void FileSystem::setBasePath(const Path& path)
{
    sBasePath = path;
}

const Path& FileSystem::getCurrentPath()
{
    if (sPathStack.empty()) {
        return sBasePath;
    }

    return sPathStack.back();
}

void FileSystem::pushPath(const Path& path)
{
    Path filePath(path);
    if (!filePath.is_absolute()) {
        filePath = sBasePath / filePath;
    }

    sPathStack.push_back(filePath);
}

void FileSystem::popPath()
{
    if (!sPathStack.empty()) {
        sPathStack.pop_back();
    }
}

bool FileSystem::exist(const Path& path)
{
    return std::filesystem::exists(path);
}

Path FileSystem::convertPath(const Path& path)
{
    Path filePath(path);
    if (!filePath.is_absolute()) {
        filePath = getCurrentPath() / filePath;
    }

    return absolute(filePath);
}

std::vector<uint8_t> FileSystem::readAll(const Path& path) noexcept
{
    Path filePath(path);
    if (!filePath.is_absolute()) {
        filePath = getCurrentPath() / filePath;
    }

    File file(filePath.string(), FileMode::Read);
    if (!file.isOpen()) {
        return {};
    }

    auto fileSize = file.getSize();
    std::vector<uint8_t> bytes;
    bytes.resize(fileSize);
    file.read(bytes.data(), fileSize);
    return bytes;
}

std::string FileSystem::readAllText(const Path& path) noexcept
{
    Path filePath(path);
    if (!filePath.is_absolute()) {
        filePath = getCurrentPath() / filePath;
    }

    File file(filePath.string(), FileMode::Read);
    if (!file.isOpen()) {
        return "";
    }

    auto fileSize = file.getSize();
    std::string str;
    str.resize(fileSize);
    file.read(str.data(), fileSize);
    return str;
}

} // namespace mygfx
