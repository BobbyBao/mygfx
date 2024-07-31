#include "FileUtils.h"

namespace utils {

	std::vector<uint8_t> FileUtils::readAll(const std::string& path) noexcept {
		if (readFileFn) {
			return readFileFn(path);
		}
		return {};
	}

	std::string FileUtils::readAllText(const std::string& path) noexcept {
		if (readTextFn) {
			return readTextFn(path);
		}
		return "";
	}
	
}