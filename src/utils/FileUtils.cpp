#include "FileUtils.h"

namespace utils {
	std::vector<Path> sPathStack;

	void FileUtils::setBasePath(const Path& path) {
		sBasePath = path;
	}
	
	const Path& FileUtils::getCurrentPath() {
		if (sPathStack.empty()) {
			return sBasePath;
		}

		return sPathStack.back();
	} 
	
	void FileUtils::pushPath(const Path& path) {
		Path filePath(path);
		if (!filePath.is_absolute()) {
			filePath = sBasePath / filePath; 
		}

		sPathStack.push_back(filePath);
	}
			
	void FileUtils::popPath() {
		if (!sPathStack.empty()) {
			sPathStack.pop_back();
		}
	}

	bool FileUtils::exist(const Path& path) {
		if (sIOStream.exist) {
			return sIOStream.exist(path);
		}
		return false;
	}

	Path FileUtils::convertPath(const Path& path) {
		Path filePath(path);
		if (!filePath.is_absolute()) {
			filePath = getCurrentPath() / filePath;
		}

		return absolute(filePath);
	}

	std::vector<uint8_t> FileUtils::readAll(const Path& path) noexcept {
		Path filePath(path);
		if (!filePath.is_absolute()) {
			filePath = getCurrentPath() / filePath; 
		}

		if (sIOStream.readAll) {
			return sIOStream.readAll(filePath.string());
		}

		return {};
	}

	std::string FileUtils::readAllText(const Path& path) noexcept {	
		Path filePath(path);
		if (!filePath.is_absolute()) {
			filePath = getCurrentPath() / filePath; 
		}

		if (sIOStream.readAllText) {
			return sIOStream.readAllText(filePath);
		}

		return "";
	}
	
}