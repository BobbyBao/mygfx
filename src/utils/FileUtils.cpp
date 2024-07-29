#include "FileUtils.h"

namespace utils {
	
	std::string FileUtils::readAllText(const std::string& path) noexcept {
		if (readFileFn) {
			return readFileFn(path);
		}
		return "";
	}
	
}