#pragma once
#include "Fwd.h"
#include <filesystem>
#include "io/Stream.h"

namespace mygfx {


class FileSystem {
public:
    static void setBasePath(const Path& path);
    static const Path& getBasePath();
    static bool exist(const Path& path);
    static Path convertPath(const Path& path);
    static std::vector<uint8_t> readAll(const Path& path) noexcept;
    static std::string readAllText(const Path& path) noexcept;
};

} // namespace mygfx
