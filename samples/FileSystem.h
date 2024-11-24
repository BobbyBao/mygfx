#pragma once

#include <filesystem>
#include <fstream>

namespace mygfx {

using Path = std::filesystem::path;

class FileSystem {
    inline static Path sBasePath;
    inline static Vector<Path> sSearchPath;
public:
    static void setBasePath(const Path& filePath)
    {
        sBasePath = normalizeDir(filePath);
        sSearchPath.push_back(filePath);
    }
    
    static void addSearchPath(const Path& filePath)
    {     
        Path path = filePath;
        if (!path.is_absolute()) {
            path = sBasePath / filePath;
        }

        sSearchPath.push_back(normalizeDir(path));
    }

    static String readAllText(const Path& filePath)
    {
        std::ifstream is(filePath.string(), std::ios::binary | std::ios::in | std::ios::ate);
        if (!is.is_open()) {
            return {};
        }

        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);

        String text;
        text.resize(size);
        is.read(text.data(), size);
        is.close();

        return text;
    }

    static Path normalizeDir(const Path& nameIn)
    {
        Path path = nameIn;
        if (!path.is_absolute()) {
            path = std::filesystem::absolute(path);
        }

        return path.lexically_normal();
    }

};
}