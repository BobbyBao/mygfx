#include "FileSystem.h"
#include "utils/Log.h"

namespace mygfx {

static Path sBasePath;
static std::vector<Path> sPathStack;

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

    FileStream file(filePath.string(), FileMode::Read);
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

    FileStream file(filePath.string(), FileMode::Read);
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
