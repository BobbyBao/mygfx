#pragma once
#include "VulkanDefs.h"

#include <assert.h>
#include <cstring>
#include <fstream>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif
#ifdef __ANDROID__
#include "VulkanAndroid.h"
#endif

namespace mygfx {
namespace debug {
    // Default debug callback
    VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    // Load debug function pointers and set debug callback
    void setupDebugging(VkInstance instance);
    // Clear debug callback
    void freeDebugCallback(VkInstance instance);
    // Used to populate a VkDebugUtilsMessengerCreateInfoEXT with our example messenger function and desired flags
    void setupDebugingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCI);
}

// Wrapper for the VK_EXT_debug_utils extension
// These can be used to name Vulkan objects for debugging tools like RenderDoc
namespace debugutils {
    void setup(VkInstance instance);
    void cmdBeginLabel(VkCommandBuffer cmdbuffer, std::string caption, float color[4]);
    void cmdEndLabel(VkCommandBuffer cmdbuffer);
}
}
