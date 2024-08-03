#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Fwd.h"

namespace utils {

	struct IOStream {
		bool (*exist)(const std::string&) = nullptr;
		std::vector<uint8_t> (*readAll)(const std::string&) = nullptr;
		std::string (*readAllText)(const std::string&) = nullptr;
	};

	class FileUtils {
	public:
		inline static IOStream sIOStream;
		inline static HashSet<std::string> sSearchPath;
		static void addSearchPath(const std::string& path);
		static void removeSearchPath(const std::string& path);
		static bool exist(const std::string& path);
		static std::vector<uint8_t> readAll(const std::string& path) noexcept;
		static std::string readAllText(const std::string& path) noexcept;
	};
}