#pragma once
#include <string>
#include <vector>
#include <functional>
#include "GraphicsFwd.h"
#include <filesystem>

namespace utils {
	
	using Path = std::filesystem::path;

	struct IOStream {
		bool (*exist)(const Path&) = nullptr;
		std::vector<uint8_t>(*readAll)(const Path&) = nullptr;
		std::string(*readAllText)(const Path&) = nullptr;
	};

	class FileUtils {
	public:
		inline static IOStream sIOStream;
		inline static Path sBasePath;
		static void setBasePath(const Path& path);
		static const Path& getCurrentPath();
		static void pushPath(const Path& path);
		static void popPath();
		static bool exist(const Path& path);
		static Path convertPath(const Path& path);
		static std::vector<uint8_t> readAll(const Path& path) noexcept;
		static std::string readAllText(const Path& path) noexcept;
	};
}