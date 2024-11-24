#pragma once

#include <filesystem>

namespace mygfx {

using Path = std::filesystem::path;

class FileSystem {
public:

    inline static Path sBasePath;

    static void setBasePath(const Path& filePath)
    {
        sBasePath = filePath;
    }

    static String readAllText(const Path& filePath)
    {
        return {};
    }
};
}