#include "VulkanHelper.h"
#include "VulkanDevice.h"
#include "utils/Log.h"

namespace mygfx {

#define GET_INSTANCE_PROC_ADDR(name) g_##name = (PFN_##name)vkGetInstanceProcAddr(devide, #name);
#define GET_DEVICE_PROC_ADDR(name) g_##name = (PFN_##name)vkGetDeviceProcAddr(device, #name);

static VkPhysicalDeviceBufferDeviceAddressFeatures BufferDeviceAddressFeatures = {};
static VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexingFeatures = {};

PFN_vkSetDebugUtilsObjectNameEXT g_vkSetDebugUtilsObjectNameEXT = nullptr;

#if HAS_SHADER_OBJECT_EXT
PFN_vkCreateShadersEXT g_vkCreateShadersEXT { VK_NULL_HANDLE };
PFN_vkDestroyShaderEXT g_vkDestroyShaderEXT { VK_NULL_HANDLE };
PFN_vkCmdBindShadersEXT g_vkCmdBindShadersEXT { VK_NULL_HANDLE };
PFN_vkGetShaderBinaryDataEXT g_vkGetShaderBinaryDataEXT { VK_NULL_HANDLE };
#endif
// VK_EXT_shader_objects requires render passes to be dynamic
PFN_vkCmdBeginRenderingKHR g_vkCmdBeginRenderingKHR { VK_NULL_HANDLE };
PFN_vkCmdEndRenderingKHR g_vkCmdEndRenderingKHR { VK_NULL_HANDLE };

// With VK_EXT_shader_object pipeline state must be set at command buffer creation using these functions
PFN_vkCmdSetAlphaToCoverageEnableEXT g_vkCmdSetAlphaToCoverageEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetColorBlendEnableEXT g_vkCmdSetColorBlendEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetColorBlendEquationEXT g_vkCmdSetColorBlendEquationEXT { VK_NULL_HANDLE };
PFN_vkCmdSetColorWriteMaskEXT g_vkCmdSetColorWriteMaskEXT { VK_NULL_HANDLE };
PFN_vkCmdSetCullModeEXT g_vkCmdSetCullModeEXT { VK_NULL_HANDLE };
PFN_vkCmdSetDepthBiasEnableEXT g_vkCmdSetDepthBiasEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetDepthCompareOpEXT g_vkCmdSetDepthCompareOpEXT { VK_NULL_HANDLE };
PFN_vkCmdSetDepthTestEnableEXT g_vkCmdSetDepthTestEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetDepthWriteEnableEXT g_vkCmdSetDepthWriteEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetFrontFaceEXT g_vkCmdSetFrontFaceEXT { VK_NULL_HANDLE };
PFN_vkCmdSetPolygonModeEXT g_vkCmdSetPolygonModeEXT { VK_NULL_HANDLE };
PFN_vkCmdSetPrimitiveRestartEnableEXT g_vkCmdSetPrimitiveRestartEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetPrimitiveTopologyEXT g_vkCmdSetPrimitiveTopologyEXT { VK_NULL_HANDLE };
PFN_vkCmdSetRasterizationSamplesEXT g_vkCmdSetRasterizationSamplesEXT { VK_NULL_HANDLE };
PFN_vkCmdSetRasterizerDiscardEnableEXT g_vkCmdSetRasterizerDiscardEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetSampleMaskEXT g_vkCmdSetSampleMaskEXT { VK_NULL_HANDLE };
PFN_vkCmdSetScissorWithCountEXT g_vkCmdSetScissorWithCountEXT { VK_NULL_HANDLE };
PFN_vkCmdSetStencilTestEnableEXT g_vkCmdSetStencilTestEnableEXT { VK_NULL_HANDLE };
PFN_vkCmdSetViewportWithCountEXT g_vkCmdSetViewportWithCountEXT { VK_NULL_HANDLE };

// VK_EXT_vertex_input_dynamic_state
PFN_vkCmdSetVertexInputEXT g_vkCmdSetVertexInputEXT { VK_NULL_HANDLE };

VulkanHelper::VulkanHelper()
{
#if HAS_SHADER_OBJECT_EXT
    enabledDeviceExtensions.push_back(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
#endif
    enabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    enabledDeviceExtensions.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);

    // With VK_EXT_shader_object all baked pipeline state is set dynamically at command buffer creation, so we need to enable additional extensions
    enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    enabledDeviceExtensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);

    // Since we are not requiring Vulkan 1.2, we need to enable some additional extensios for dynamic rendering
    enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    enabledDeviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    enabledDeviceExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    enabledDeviceExtensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);

    enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
}

VkResult VulkanHelper::createInstance(const char* name, bool validation)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name;
    appInfo.pEngineName = name;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

    // Enable surface extensions depending on os
#if defined(_WIN32)
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
    instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    instanceExtensions.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
    instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    instanceExtensions.push_back(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME);
#endif

    // Get extensions supported by the instance and store for later use
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0) {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
            for (VkExtensionProperties& extension : extensions) {
                supportedInstanceExtensions.push_back(extension.extensionName);
            }
        }
    }

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
    // SRS - When running on iOS/macOS with MoltenVK, enable VK_KHR_get_physical_device_properties2 if not already enabled by the example (required by VK_KHR_portability_subset)
    if (std::find(enabledInstanceExtensions.begin(), enabledInstanceExtensions.end(), VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == enabledInstanceExtensions.end()) {
        enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
#endif

    // Enabled requested instance extensions
    if (enabledInstanceExtensions.size() > 0) {
        for (const char* enabledExtension : enabledInstanceExtensions) {
            // Output message if requested extension is not available
            if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end()) {
                std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
            }
            instanceExtensions.push_back(enabledExtension);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI {};
    if (validation) {
        debug::setupDebugingMessengerCreateInfo(debugUtilsMessengerCI);
        debugUtilsMessengerCI.pNext = instanceCreateInfo.pNext;
        instanceCreateInfo.pNext = &debugUtilsMessengerCI;
    }

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)) && defined(VK_KHR_portability_enumeration)
    // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_enumeration is defined and supported by the instance, enable the extension and the flag
    if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
        instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    // Enable the debug utils extension if available (e.g. when debugging tools are present)
    if (validation || std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (instanceExtensions.size() > 0) {
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    }

    // The VK_LAYER_KHRONOS_validation contains all current validation functionality.
    // Note that on Android this layer requires at least NDK r20
    const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
    if (validation) {
        // Check if this layer is available at instance level
        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
        bool validationLayerPresent = false;
        for (VkLayerProperties& layer : instanceLayerProperties) {
            if (strcmp(layer.layerName, validationLayerName) == 0) {
                validationLayerPresent = true;
                break;
            }
        }
        if (validationLayerPresent) {
            instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
            instanceCreateInfo.enabledLayerCount = 1;
        } else {
            std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
        }
    }

    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    // If the debug utils extension is present we set up debug functions, so samples can label objects for debugging
    if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
        debugutils::setup(instance);
    }

    if (VK_SUCCESS == result) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        android::loadVulkanFunctions(instance);
#endif

        // If requested, we enable the default validation layers for debugging
        if (validation) {
            debug::setupDebugging(instance);
        }
    }

    return result;
}

bool VulkanHelper::selectPhysicalDevice()
{
    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
    if (gpuCount == 0) {
        tools::exitFatal("No device with Vulkan support found", -1);
        return false;
    }
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    auto err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
    if (err) {
        tools::exitFatal("Could not enumerate physical devices : \n" + tools::errorString(err), err);
        return false;
    }

    // GPU selection

    // Select physical device to be used for the Vulkan example
    // Defaults to the first device unless specified by command line
    uint32_t selectedDevice = 0;

    physicalDevice = physicalDevices[selectedDevice];

    // Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    // Queue family properties, used for setting up requested queues upon device creation
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    assert(queueFamilyCount > 0);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    // Get list of supported extensions
    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
    if (extCount > 0) {
        supportedExtensions.resize(extCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &supportedExtensions.front());
    }

    return true;
}

void VulkanHelper::getEnabledFeatures()
{
    featuresAppender.AppendNext(&enabledShaderObjectFeaturesEXT, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT);
    featuresAppender.AppendNext(&enabledDynamicRenderingFeaturesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR);

    if (tryAddExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)) {
        featuresAppender.AppendNext(&DescriptorIndexingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES);
    }

    // VK_KHR_timeline_semaphore was promoted to 1.2, so no need to query the extension
    // this is needed for timeline semaphore
    static VkPhysicalDeviceTimelineSemaphoreFeaturesKHR semaphoreFeatures = {};
    featuresAppender.AppendNext(&semaphoreFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR);

    if (tryAddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)) {
        featuresAppender.AppendNext(&BufferDeviceAddressFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES);
    }

    if (tryAddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) {
        static VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .extendedDynamicState = VK_TRUE,
        };
        featuresAppender.AppendNext(&features);
    }

    if (tryAddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
        static VkPhysicalDeviceExtendedDynamicState2FeaturesEXT features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
            .extendedDynamicState2 = VK_TRUE,
            .extendedDynamicState2PatchControlPoints = VK_TRUE
        };
        featuresAppender.AppendNext(&features);
    }

    if (tryAddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)) {
        static VkPhysicalDeviceExtendedDynamicState3FeaturesEXT features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
            .extendedDynamicState3DepthClampEnable = VK_TRUE,
            .extendedDynamicState3PolygonMode = VK_TRUE
        };
        featuresAppender.AppendNext(&features);
    }

    if (tryAddExtension(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)) {
        static VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
            .vertexInputDynamicState = VK_TRUE,
        };
        featuresAppender.AppendNext(&features);
    }

}

void VulkanHelper::getEnabledExtensions()
{
}

uint32_t VulkanHelper::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                if (memTypeFound) {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound) {
        *memTypeFound = false;
        return 0;
    } else {
        throw std::runtime_error("Could not find a matching memory type");
    }
}

uint32_t VulkanHelper::getQueueFamilyIndex(VkQueueFlags queueFlags) const
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
        if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
            return i;
        }
    }

    throw std::runtime_error("Could not find a matching queue family index");
}

VkResult VulkanHelper::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, bool useSwapChain, VkQueueFlags requestedQueueTypes)
{
    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
    // requests different queue types

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos {};

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation

    float graphicsQueuePriority[] = { 1.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    float computeQueuePriority = 1.0f;
    float copyQueuePriorities[] = { 1.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueInfo {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        queueInfo.queueCount = std::max(8u, queueFamilyProperties[queueFamilyIndices.graphics].queueCount);
        queueInfo.pQueuePriorities = graphicsQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    } else {
        queueFamilyIndices.graphics = 0;
    }

    // Dedicated compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
        queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
            queueInfo.queueCount = std::max(8u, queueFamilyProperties[queueFamilyIndices.compute].queueCount);
            queueInfo.pQueuePriorities = &computeQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    } else {
        // Else we use the same queue
        queueFamilyIndices.compute = queueFamilyIndices.graphics;
    }

    // Dedicated transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
        queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute)) {
            // If transfer family index differs, we need an additional queue create info for the transfer queue
            VkDeviceQueueCreateInfo queueInfo {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
            queueInfo.queueCount = std::max(8u, queueFamilyProperties[queueFamilyIndices.transfer].queueCount);;
            queueInfo.pQueuePriorities = copyQueuePriorities;
            queueCreateInfos.push_back(queueInfo);
        }
    } else {
        // Else we use the same queue
        queueFamilyIndices.transfer = queueFamilyIndices.graphics;
    }

    // Create the logical device representation
    std::vector<const char*> deviceExtensions(enabledExtensions);
    if (useSwapChain) {
        // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    ;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = featuresAppender.GetNext();
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);

    // If a pNext(Chain) has been passed, we need to add it to the device creation info
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 {};
    if (featuresAppender.GetNext()) {
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceFeatures2.features = enabledFeatures;
        physicalDeviceFeatures2.pNext = featuresAppender.GetNext();
        deviceCreateInfo.pEnabledFeatures = nullptr;
        deviceCreateInfo.pNext = &physicalDeviceFeatures2;
    }

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)) && defined(VK_KHR_portability_subset)
    // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_subset is defined and supported by the device, enable the extension
    if (extensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
#endif

    if (deviceExtensions.size() > 0) {
        for (const char* enabledExtension : deviceExtensions) {
            if (!extensionSupported(enabledExtension)) {
                std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
            }
        }

        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    }

    this->enabledFeatures = enabledFeatures;

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        return result;
    }

    GET_DEVICE_PROC_ADDR(vkSetDebugUtilsObjectNameEXT);
    
#if HAS_SHADER_OBJECT_EXT
    g_vkCreateShadersEXT = reinterpret_cast<PFN_vkCreateShadersEXT>(vkGetDeviceProcAddr(device, "vkCreateShadersEXT"));
    g_vkDestroyShaderEXT = reinterpret_cast<PFN_vkDestroyShaderEXT>(vkGetDeviceProcAddr(device, "vkDestroyShaderEXT"));
    g_vkCmdBindShadersEXT = reinterpret_cast<PFN_vkCmdBindShadersEXT>(vkGetDeviceProcAddr(device, "vkCmdBindShadersEXT"));
    g_vkGetShaderBinaryDataEXT = reinterpret_cast<PFN_vkGetShaderBinaryDataEXT>(vkGetDeviceProcAddr(device, "vkGetShaderBinaryDataEXT"));
#endif

    g_vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR"));
    g_vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR"));

    g_vkCmdSetAlphaToCoverageEnableEXT = reinterpret_cast<PFN_vkCmdSetAlphaToCoverageEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetAlphaToCoverageEnableEXT"));
    g_vkCmdSetColorBlendEnableEXT = reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEnableEXT"));

    GET_DEVICE_PROC_ADDR(vkCmdSetColorBlendEquationEXT)

    g_vkCmdSetColorWriteMaskEXT = reinterpret_cast<PFN_vkCmdSetColorWriteMaskEXT>(vkGetDeviceProcAddr(device, "vkCmdSetColorWriteMaskEXT"));
    g_vkCmdSetCullModeEXT = reinterpret_cast<PFN_vkCmdSetCullModeEXT>(vkGetDeviceProcAddr(device, "vkCmdSetCullModeEXT"));
    g_vkCmdSetDepthBiasEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthBiasEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetDepthBiasEnableEXT"));
    g_vkCmdSetDepthCompareOpEXT = reinterpret_cast<PFN_vkCmdSetDepthCompareOpEXT>(vkGetDeviceProcAddr(device, "vkCmdSetDepthCompareOpEXT"));
    g_vkCmdSetDepthTestEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthTestEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetDepthTestEnableEXT"));
    g_vkCmdSetDepthWriteEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthWriteEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetDepthWriteEnableEXT"));
    g_vkCmdSetFrontFaceEXT = reinterpret_cast<PFN_vkCmdSetFrontFaceEXT>(vkGetDeviceProcAddr(device, "vkCmdSetFrontFaceEXT"));
    g_vkCmdSetPolygonModeEXT = reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT"));
    g_vkCmdSetPrimitiveRestartEnableEXT = reinterpret_cast<PFN_vkCmdSetPrimitiveRestartEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveRestartEnableEXT"));
    g_vkCmdSetPrimitiveTopologyEXT = reinterpret_cast<PFN_vkCmdSetPrimitiveTopologyEXT>(vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveTopologyEXT"));
    g_vkCmdSetRasterizationSamplesEXT = reinterpret_cast<PFN_vkCmdSetRasterizationSamplesEXT>(vkGetDeviceProcAddr(device, "vkCmdSetRasterizationSamplesEXT"));
    g_vkCmdSetRasterizerDiscardEnableEXT = reinterpret_cast<PFN_vkCmdSetRasterizerDiscardEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetRasterizerDiscardEnableEXT"));
    g_vkCmdSetSampleMaskEXT = reinterpret_cast<PFN_vkCmdSetSampleMaskEXT>(vkGetDeviceProcAddr(device, "vkCmdSetSampleMaskEXT"));
    g_vkCmdSetScissorWithCountEXT = reinterpret_cast<PFN_vkCmdSetScissorWithCountEXT>(vkGetDeviceProcAddr(device, "vkCmdSetScissorWithCountEXT"));
    g_vkCmdSetStencilTestEnableEXT = reinterpret_cast<PFN_vkCmdSetStencilTestEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetStencilTestEnableEXT"));
    g_vkCmdSetVertexInputEXT = reinterpret_cast<PFN_vkCmdSetVertexInputEXT>(vkGetDeviceProcAddr(device, "vkCmdSetVertexInputEXT"));
    g_vkCmdSetViewportWithCountEXT = reinterpret_cast<PFN_vkCmdSetViewportWithCountEXT>(vkGetDeviceProcAddr(device, "vkCmdSetViewportWithCountEXT"));
 
    return result;
}

void VulkanHelper::setResourceName(VkObjectType objectType, uint64_t handle, const char* name)
{
    if (g_vkSetDebugUtilsObjectNameEXT && handle && name) {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = handle;
        nameInfo.pObjectName = name;
        g_vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
    }
}

bool VulkanHelper::tryAddExtension(const char* pExtensionName)
{
    if (extensionSupported(pExtensionName)) {
        enabledDeviceExtensions.push_back(pExtensionName);
        return true;
    } else {
        LOG_WARNING("Extension {} not found", pExtensionName);
        return false;
    }
}

bool VulkanHelper::extensionSupported(const char* pExtensionName)
{
    return std::find_if(
               supportedExtensions.begin(), supportedExtensions.end(),
               [pExtensionName](const VkExtensionProperties& extensionProps) -> bool {
                   return strcmp(extensionProps.extensionName, pExtensionName) == 0;
               })
        != supportedExtensions.end();
}

VkFormat VulkanHelper::getSupportedDepthFormat(bool checkSamplingSupport)
{
    // All depth formats may be optional, so we need to find a suitable depth format to use
    std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    for (auto& format : depthFormats) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            if (checkSamplingSupport) {
                if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
                    continue;
                }
            }
            return format;
        }
    }
    throw std::runtime_error("Could not find a matching depth format");
}

}