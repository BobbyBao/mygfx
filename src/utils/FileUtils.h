#pragma once
#include <string>

namespace utils {
	
class FileUtils {
public:
    static std::string readAllText(const std::string& path) noexcept;
};
}