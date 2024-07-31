#pragma once
#include <string>
#include <vector>
#include <functional>

namespace utils {

	class FileUtils {
	public:
		inline static std::function<std::string(const std::string&)> readTextFn;
		inline static std::function<std::vector<uint8_t>(const std::string&)> readFileFn;
		static std::vector<uint8_t> readAll(const std::string& path) noexcept;
		static std::string readAllText(const std::string& path) noexcept;
	};
}