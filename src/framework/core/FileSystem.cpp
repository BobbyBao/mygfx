#include "FileSystem.h"
#include "utils/Log.h"

namespace mygfx {

static Path sBasePath;

void FileSystem::setBasePath(const Path& path)
{
    sBasePath = path;
}

const Path& FileSystem::getBasePath()
{        
    return sBasePath;
}

bool FileSystem::exist(const Path& path)
{
    return std::filesystem::exists(path);
}

Path FileSystem::convertPath(const Path& path)
{
    Path filePath(path);
    if (!filePath.is_absolute()) {
        filePath = getBasePath() / filePath;
    }

    return absolute(filePath);
}

std::vector<uint8_t> FileSystem::readAll(const Path& path) noexcept
{
    Path filePath(path);
    if (!filePath.is_absolute()) {
        filePath = getBasePath() / filePath;
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
        filePath = getBasePath() / filePath;
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
