#include "FileUtils.h"

namespace utils {
	
	void FileUtils::addSearchPath(const std::string& path) {

	}

	bool FileUtils::exist(const std::string& path) {
		if (sIOStream.exist) {
			return sIOStream.exist(path);
		}
		return false;
	}

	std::vector<uint8_t> FileUtils::readAll(const std::string& path) noexcept {
		if (sIOStream.readAll) {
			return sIOStream.readAll(path);
		}
		return {};
	}

	std::string FileUtils::readAllText(const std::string& path) noexcept {
		if (sIOStream.readAllText) {
			return sIOStream.readAllText(path);
		}
		return "";
	}
	
}