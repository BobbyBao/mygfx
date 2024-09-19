#include "Stream.h"
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

FileStream::FileStream()
    : mMode(FileMode::Read)
    , mHandle(0)
    , mReadSyncNeeded(false)
    , mWriteSyncNeeded(false)
{
}

FileStream::FileStream(const String& fileName, FileMode mode)
    : mMode(FileMode::Read)
    , mHandle(0)
    , mReadSyncNeeded(false)
    , mWriteSyncNeeded(false)
{
    open(fileName, mode);
}

FileStream::~FileStream()
{
    close();
}

size_t FileStream::getSize() const
{
    return mSize;
}

bool FileStream::open(const String& fileName, FileMode mode)
{
    return openInternal(fileName, mode);
}

size_t FileStream::read(void* dest, size_t size)
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

size_t FileStream::seek(size_t position)
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

size_t FileStream::write(const void* data, size_t size)
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

void FileStream::close()
{
    if (mHandle) {
        fclose((FILE*)mHandle);
        mHandle = 0;
        mPosition = 0;
        mSize = 0;
    }
}

void FileStream::flush()
{
    if (mHandle) {
        fflush((FILE*)mHandle);
    }
}

bool FileStream::isOpen() const
{
    return mHandle != nullptr;
}

bool FileStream::openInternal(const String& fileName, FileMode mode)
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

    mSize = (unsigned)size;
    mName = fileName;
    mMode = mode;
    mPosition = 0;
    return true;
}

bool FileStream::readInternal(void* dest, size_t size)
{
    return fread(dest, size, 1, (FILE*)mHandle) == 1;
}

void FileStream::seekInternal(size_t newPosition)
{
    fseek((FILE*)mHandle, (long)newPosition, SEEK_SET);
}

}