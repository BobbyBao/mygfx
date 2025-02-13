#include "VulkanDeviceHelper.h"
#include "VulkanDebug.h"
#include "VulkanTools.h"
#include "utils/Log.h"

#if USE_VOLK
#define VOLK_IMPLEMENTATION
#include "Volk/volk.h"
#endif

#include <vulkan/vulkan_beta.h>

namespace mygfx {

#define GET_INSTANCE_PROC_ADDR(name) g_##name = (PFN_##name)vkGetInstanceProcAddr(devide, #name);
#define GET_DEVICE_PROC_ADDR(name) g_##name = (PFN_##name)vkGetDeviceProcAddr(device, #name);

static VkPhysicalDeviceBufferDeviceAddressFeatures BufferDeviceAddressFeatures = {};
static VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexingFeatures = {};

VulkanDeviceHelper::VulkanDeviceHelper()
{
    enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#if HAS_SHADER_OBJECT_EXT
    enabledDeviceExtensions.push_back(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
#endif
    enabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    enabledDeviceExtensions.push_back(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);

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

    enabledDeviceExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    // enabledDeviceExtensions.push_back(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT))
    // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_subset is defined and supported by the device, enable the extension
    enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    setenv("MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS", "1", 1);

#endif
}

bool VulkanDeviceHelper::create(const char* name, bool validation)
{
#if USE_VOLK
    volkInitialize();
#endif

    // Vulkan instance
    VkResult err = createInstance(name, validation);
    if (err) {
        tools::exitFatal("Could not create Vulkan instance : \n" + tools::errorString(err), err);
        return false;
    }

#if USE_VOLK
    volkLoadInstance(instance);
#endif

    if (!selectPhysicalDevice()) {
        return false;
    }

    getEnabledExtensions();
    getEnabledFeatures();

    VkResult res = createLogicalDevice(enabledFeatures, enabledDeviceExtensions);
    if (res != VK_SUCCESS) {
        tools::exitFatal("Could not create Vulkan device: \n" + tools::errorString(res), res);
        return false;
    }

#define VK_FUNCTION(NAME) \
    NAME = (PFN_##NAME)vkGetDeviceProcAddr(device, #NAME)

    VK_FUNCTION(vkSetDebugUtilsObjectNameEXT);

    VK_FUNCTION(vkCmdBeginRenderingKHR);
    VK_FUNCTION(vkCmdEndRenderingKHR);

    VK_FUNCTION(vkCmdSetCullModeEXT);
    VK_FUNCTION(vkCmdSetFrontFaceEXT);
    VK_FUNCTION(vkCmdSetPrimitiveTopologyEXT);

    VK_FUNCTION(vkCmdSetViewportWithCountEXT);
    VK_FUNCTION(vkCmdSetScissorWithCountEXT);

    VK_FUNCTION(vkCmdSetDepthTestEnableEXT);
    VK_FUNCTION(vkCmdSetDepthWriteEnableEXT);
    VK_FUNCTION(vkCmdSetDepthCompareOpEXT);
    VK_FUNCTION(vkCmdSetDepthBiasEnableEXT);

    VK_FUNCTION(vkCmdSetStencilTestEnableEXT);
    VK_FUNCTION(vkCmdSetRasterizerDiscardEnableEXT);
    VK_FUNCTION(vkCmdSetPrimitiveRestartEnableEXT);

#undef VK_FUNCTION

    // Get a graphics queue from the device
    vkGetDeviceQueue(device, queueFamilyIndices.graphics, 0, &queue);
    vkGetDeviceQueue(device, queueFamilyIndices.compute, 0, &computeQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.transfer, 0, &transferQueue);

    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorInfo, &mVmaAllocator);
    return true;
}

VkResult VulkanDeviceHelper::createInstance(const char* name, bool validation)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name;
    appInfo.pEngineName = name;

#if MYGFX_FEATURE_LEVEL == 1
    appInfo.apiVersion = VK_API_VERSION_1_1;
#elif MYGFX_FEATURE_LEVEL == 2
    appInfo.apiVersion = VK_API_VERSION_1_2;
#elif MYGFX_FEATURE_LEVEL == 3
    appInfo.apiVersion = VK_API_VERSION_1_3;
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

    std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

    auto supportInstanceExtension = [this](const char* ext) -> bool {
        return std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), ext) != supportedInstanceExtensions.end();
    };

    auto tryAddInstanceExtension = [supportInstanceExtension, &instanceExtensions, this](const char* ext) -> bool {
        if (supportInstanceExtension(ext)) {
            instanceExtensions.push_back(ext);
            return true;
        } else {
            LOG_WARNING("Enabled instance extension \"{}\" is not present at instance level", ext);
            return false;
        }
    };

    // Enable surface extensions depending on os
#if defined(_WIN32)
    tryAddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    tryAddInstanceExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
    tryAddInstanceExtension(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    tryAddInstanceExtension(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    tryAddInstanceExtension(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    tryAddInstanceExtension(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    tryAddInstanceExtension(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    tryAddInstanceExtension(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    tryAddInstanceExtension(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
    tryAddInstanceExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    tryAddInstanceExtension(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME);
#endif

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT))
    // SRS - When running on iOS/macOS with MoltenVK, enable VK_KHR_get_physical_device_properties2 if not already enabled by the example (required by VK_KHR_portability_subset)
    tryAddInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    // Enabled requested instance extensions
    if (enabledInstanceExtensions.size() > 0) {
        for (const char* enabledExtension : enabledInstanceExtensions) {
            // Output message if requested extension is not available
            tryAddInstanceExtension(enabledExtension);
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

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)) && defined(VK_KHR_portability_enumeration)
    // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_enumeration is defined and supported by the instance, enable the extension and the flag
    if (tryAddInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    // Enable the debug utils extension if available (e.g. when debugging tools are present)
    if (validation || supportInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
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

#if !USE_VOLK
        vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR");
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        // android::loadVulkanFunctions(instance);
#endif

        // If requested, we enable the default validation layers for debugging
        if (validation) {
            debug::setupDebugging(instance);
        }
    }

    return result;
}

bool VulkanDeviceHelper::selectPhysicalDevice()
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

void VulkanDeviceHelper::getEnabledFeatures()
{
#if HAS_SHADER_OBJECT_EXT
    enabledShaderObjectFeaturesEXT.shaderObject = true;
    featuresAppender.AppendNext(&enabledShaderObjectFeaturesEXT, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT);
#endif
    featuresAppender.AppendNext(&enabledDynamicRenderingFeaturesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR);

    if (tryAddExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)) {

        DescriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing = true;
        DescriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing = true;
        DescriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing = true;
        DescriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
        DescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
        DescriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
        DescriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = true;

        DescriptorIndexingFeatures.shaderInputAttachmentArrayNonUniformIndexing = true;
        DescriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing = true;
        DescriptorIndexingFeatures.shaderStorageTexelBufferArrayNonUniformIndexing = true;
        DescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;

        DescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
        DescriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = true;
        DescriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = true;

        DescriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = true;

        DescriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
        DescriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = true;
        DescriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
        DescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = true;
        featuresAppender.AppendNext(&DescriptorIndexingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES);

        VkPhysicalDeviceProperties2 device_properties;
        descriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;
        device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        device_properties.pNext = &descriptorIndexingProperties;
        vkGetPhysicalDeviceProperties2KHR(physicalDevice, &device_properties);

        LOG_INFO("maxPerStageDescriptorUpdateAfterBindSamplers: {}", descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSamplers);
        LOG_INFO("maxPerStageDescriptorUpdateAfterBindSampledImages: {}", descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSampledImages);
    }

    // VK_KHR_timeline_semaphore was promoted to 1.2, so no need to query the extension
    // this is needed for timeline semaphore
    static VkPhysicalDeviceTimelineSemaphoreFeaturesKHR semaphoreFeatures = {};
    featuresAppender.AppendNext(&semaphoreFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR);

    if (tryAddExtension(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)) {
        featuresAppender.AppendNext(&BufferDeviceAddressFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES);
    }

    if (tryAddExtension(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME)) {
        static VkPhysicalDeviceNestedCommandBufferFeaturesEXT features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT,
            .nestedCommandBuffer = VK_TRUE,
            .nestedCommandBufferRendering = VK_TRUE,
            .nestedCommandBufferSimultaneousUse = VK_TRUE,
        };
        featuresAppender.AppendNext(&features);
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
            .extendedDynamicState2LogicOp = VK_TRUE,
            .extendedDynamicState2PatchControlPoints = VK_TRUE
        };
        featuresAppender.AppendNext(&features);
    }

    if (tryAddExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)) {
        static VkPhysicalDeviceExtendedDynamicState3FeaturesEXT features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
            .extendedDynamicState3DepthClampEnable = VK_TRUE,
            .extendedDynamicState3PolygonMode = VK_TRUE,
            .extendedDynamicState3RasterizationSamples = VK_TRUE,
            .extendedDynamicState3SampleMask = VK_TRUE,
            .extendedDynamicState3AlphaToCoverageEnable = VK_TRUE,
            .extendedDynamicState3AlphaToOneEnable = VK_TRUE,
            .extendedDynamicState3LogicOpEnable = VK_TRUE,
            .extendedDynamicState3ColorBlendEnable = VK_TRUE,
            .extendedDynamicState3ColorBlendEquation = VK_TRUE,
            .extendedDynamicState3ColorWriteMask = VK_TRUE,
            .extendedDynamicState3RasterizationStream = VK_TRUE,
            .extendedDynamicState3ConservativeRasterizationMode = VK_TRUE,
            .extendedDynamicState3ExtraPrimitiveOverestimationSize = VK_TRUE,
            .extendedDynamicState3DepthClipEnable = VK_TRUE,
            .extendedDynamicState3SampleLocationsEnable = VK_TRUE,
            .extendedDynamicState3ColorBlendAdvanced = VK_TRUE,
            .extendedDynamicState3ProvokingVertexMode = VK_TRUE,
            .extendedDynamicState3LineRasterizationMode = VK_TRUE,
            .extendedDynamicState3LineStippleEnable = VK_TRUE,
            .extendedDynamicState3DepthClipNegativeOneToOne = VK_TRUE,
            .extendedDynamicState3ViewportWScalingEnable = VK_TRUE,
            .extendedDynamicState3ViewportSwizzle = VK_TRUE,
            .extendedDynamicState3CoverageToColorEnable = VK_TRUE,
            .extendedDynamicState3CoverageToColorLocation = VK_TRUE,
            .extendedDynamicState3CoverageModulationMode = VK_TRUE,
            .extendedDynamicState3CoverageModulationTableEnable = VK_TRUE,
            .extendedDynamicState3CoverageModulationTable = VK_TRUE,
            .extendedDynamicState3CoverageReductionMode = VK_TRUE,
            .extendedDynamicState3RepresentativeFragmentTestEnable = VK_TRUE,
            .extendedDynamicState3ShadingRateImageEnable = VK_TRUE,
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

uint32_t VulkanDeviceHelper::getMaxVariableCount(VkDescriptorType type) const
{
    switch (type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
        return std::min(SAMPLER_VARIABLE_DESC_COUNT, getMaxSamplers());
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return std::min(IMAGE_VARIABLE_DESC_COUNT, getMaxSampledImages());
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return std::min(IMAGE_VARIABLE_DESC_COUNT, getMaxSampledImages());
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return std::min(IMAGE_VARIABLE_DESC_COUNT, getMaxStorageImages());    
    default:
        break;
    }
    return IMAGE_VARIABLE_DESC_COUNT;
}

void VulkanDeviceHelper::getEnabledExtensions()
{
    for (auto it = enabledDeviceExtensions.begin(); it != enabledDeviceExtensions.end();) {
        if (!extensionSupported(*it)) {
            it = enabledDeviceExtensions.erase(it);
        } else {
            ++it;
        }
    }
}

uint32_t VulkanDeviceHelper::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
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

uint32_t VulkanDeviceHelper::getQueueFamilyIndex(VkQueueFlags queueFlags) const
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

VkResult VulkanDeviceHelper::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, bool useSwapChain, VkQueueFlags requestedQueueTypes)
{
    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
    // requests different queue types

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos {};

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation

    float graphicsQueuePriority[] = { 1.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    float computeQueuePriority[] = { 1.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
    float copyQueuePriorities[] = { 1.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueInfo {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        queueInfo.queueCount = std::min(8u, queueFamilyProperties[queueFamilyIndices.graphics].queueCount);
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
            queueInfo.queueCount = std::min(8u, queueFamilyProperties[queueFamilyIndices.compute].queueCount);
            queueInfo.pQueuePriorities = computeQueuePriority;
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
            queueInfo.queueCount = std::min(8u, queueFamilyProperties[queueFamilyIndices.transfer].queueCount);
            ;
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

    if (deviceExtensions.size() > 0) {
        LOG_DEBUG("Enabled device extensions:");
        for (const char* enabledExtension : deviceExtensions) {
            LOG_DEBUG(enabledExtension);
        }

        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    }

    this->enabledFeatures = enabledFeatures;

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        return result;
    }

    return result;
}

void VulkanDeviceHelper::setResourceName(VkObjectType objectType, uint64_t handle, const char* name)
{
    if (vkSetDebugUtilsObjectNameEXT && handle && name) {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.objectHandle = handle;
        nameInfo.pObjectName = name;
        vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
    }
}

bool VulkanDeviceHelper::tryAddExtension(const char* pExtensionName)
{
    if (extensionSupported(pExtensionName)) {
        enabledDeviceExtensions.push_back(pExtensionName);
        return true;
    } else {
        LOG_WARNING("Extension {} not found", pExtensionName);
        return false;
    }
}

bool VulkanDeviceHelper::extensionSupported(const char* pExtensionName)
{
    return std::find_if(
               supportedExtensions.begin(), supportedExtensions.end(),
               [pExtensionName](const VkExtensionProperties& extensionProps) -> bool {
                   return strcmp(extensionProps.extensionName, pExtensionName) == 0;
               })
        != supportedExtensions.end();
}

VkFormat VulkanDeviceHelper::getSupportedDepthFormat(bool checkSamplingSupport)
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

void VulkanDeviceHelper::destroy()
{
    vmaDestroyAllocator(mVmaAllocator);
    mVmaAllocator = NULL;

    if (device) {
        vkDestroyDevice(device, nullptr);
    }

    debug::freeDebugCallback(instance);

    vkDestroyInstance(instance, nullptr);
}

}