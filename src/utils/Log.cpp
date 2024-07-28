#include "Log.h"

#include <algorithm>
#include <mutex>
#include <stdarg.h>

#if defined(__ANDROID__)
#include <android/configuration.h>
#include <android/log.h>
#include <memory>

#else
#if defined(__WIN32__) || defined(__WINRT__)
#include <Windows.h>
#endif
#include <iostream>
#endif

#ifdef _MSC_VER 
#include <Windows.h>
#endif

namespace utils {

const char* logLevelPrefixes[] = {
    "[DEBUG] ",
    "[INFO ] ",
    "[WARN ] ",
    "[ERROR] ",
    ""
};

#if defined(__ANDROID__)
inline int logLevelToAndroidPriority(LogLevel level)
{
    switch (level) {
    case LogLevel::Debug:
        return ANDROID_LOG_DEBUG;
    case LogLevel::eInfo:
        return ANDROID_LOG_INFO;
    case LogLevel::eWarn:
        return ANDROID_LOG_WARN;
    case LogLevel::eError:
        return ANDROID_LOG_ERROR;
    }
}
#endif

void Log::write(LogLevel level, const std::string& msg)
{
    write(level, msg.c_str());
}

void Log::write(LogLevel level, const char* msg)
{
#if defined(__ANDROID__)
    int prio = logLevelToAndroidPriority(level);
    __android_log_print(prio, "mygfx", "%s", msg);
#else
#ifdef _MSC_VER
    OutputDebugStringA(logLevelPrefixes[(int)level]);
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
#endif
    std::cout << logLevelPrefixes[(int)level] << msg << std::endl;
#endif

    if (onWriteFn != nullptr) {
        onWriteFn(level, msg);
    }
}

}