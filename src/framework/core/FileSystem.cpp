
#include "FileSystem.h"


namespace mygfx {

	
std::vector<Path> sPathStack;

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
    if (sIOStream.exist) {
        return sIOStream.exist(path);
    }
    return false;
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

    if (sIOStream.readAll) {
        return sIOStream.readAll(filePath.string());
    }

    return {};
}

std::string FileSystem::readAllText(const Path& path) noexcept
{
    Path filePath(path);
    if (!filePath.is_absolute()) {
        filePath = getCurrentPath() / filePath;
    }

    if (sIOStream.readAllText) {
        return sIOStream.readAllText(filePath);
    }

    return "";
}

} // namespace utils
