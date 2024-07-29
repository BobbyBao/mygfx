#pragma once
#include <string>
#include <vector>
#include <functional>

namespace utils {

	class FileUtils {
	public:
		inline static std::function<std::string(const std::string&)> readFileFn;
		static std::string readAllText(const std::string& path) noexcept;
	};
}