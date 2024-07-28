#pragma once
#include <string>
#include <sstream>
#include <format>

#include <assert.h>

#define UTILS_LOGGING

namespace utils {

	enum class LogLevel {
		Debug = 0,
		Info,
		Warn,
		Error,
		None
	};

	class Log {
	public:
		static void write(LogLevel level, const std::string& msg);
		static void write(LogLevel level, const char* msg);

		template <typename... Args>
		static void logMessageF(LogLevel level, std::string_view fmt, Args&&... args)
		{
			std::string str = std::vformat(fmt, std::make_format_args(args...));
			//std::vformat_to(std::back_inserter(str), fmt, fmt::make_format_args(args...));
			write(level, str.c_str());
		}

		static void logMessageF(LogLevel level, std::string_view str)
		{
			write(level, str.data());
		}

		inline static void(*onWriteFn)(LogLevel level, const char* msg) = nullptr;
	private:

	};

}

#ifdef UTILS_LOGGING

#define LOG_RAW(format, ...) utils::Log::logMessageF(utils::LogLevel::None, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) utils::Log::logMessageF(utils::LogLevel::Debug, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) utils::Log::logMessageF(utils::LogLevel::Info, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) utils::Log::logMessageF(utils::LogLevel::Warn, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) utils::Log::logMessageF(utils::LogLevel::Error, format, ##__VA_ARGS__)

#else

#define LOG_RAW(...) ((void)0)
#define LOG_DEBUG(...) ((void)0)
#define LOG_INFO(...) ((void)0)
#define LOG_WARNING(...) ((void)0)
#define LOG_ERROR(...) ((void)0)

#endif