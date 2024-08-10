#pragma once
#include "VulkanDefs.h"
#include <stdint.h>
#include <string>
#include <vector>

namespace mygfx {

class VulkanHelper {
public:
    VulkanHelper();
    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
    uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;
    VkResult createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

    void setResourceName(VkObjectType objectType, uint64_t handle, const char* name);

    VkResult createVkBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, const void* data = nullptr);
    // void            copyBuffer(VulkanBuffer* src, VulkanBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);
    VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

    bool tryAddExtension(const char* pExtensionName);
    bool extensionSupported(const char* extension);
    VkFormat getSupportedDepthFormat(bool checkSamplingSupport);

    VkInstance instance { VK_NULL_HANDLE };
    VkDevice device { VK_NULL_HANDLE };
    VkPhysicalDevice physicalDevice { VK_NULL_HANDLE };
    VkQueue queue { VK_NULL_HANDLE };
    VkQueue computeQueue { VK_NULL_HANDLE };
    VkQueue transferQueue { VK_NULL_HANDLE };
    VkPhysicalDeviceFeatures enabledFeatures {};

    // Physical device (GPU) that Vulkan will use
    // Stores physical device properties (for e.g. checking device limits)
    VkPhysicalDeviceProperties properties {};
    // Stores the features available on the selected physical device (for e.g. checking if a feature is available)
    VkPhysicalDeviceFeatures features {};
    // Stores all available memory (type) properties for the physical device
    VkPhysicalDeviceMemoryProperties memoryProperties {};

    /** @brief Queue family properties of the physical device */
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    /** @brief List of extensions supported by the device */
    std::vector<VkExtensionProperties> supportedExtensions;
    /** @brief Default command pool for the graphics queue family index */
    VkCommandPool commandPool = VK_NULL_HANDLE;
    /** @brief Contains queue family indices */
    struct
    {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    } queueFamilyIndices;

protected:
    VkResult createInstance(const char* name, bool validation);
    bool selectPhysicalDevice();
    void getEnabledFeatures();
    void getEnabledExtensions();

    std::vector<std::string> supportedInstanceExtensions;

    /** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
    std::vector<const char*> enabledDeviceExtensions;
    std::vector<const char*> enabledInstanceExtensions;

    VkPhysicalDeviceShaderObjectFeaturesEXT enabledShaderObjectFeaturesEXT {};
    VkPhysicalDeviceDynamicRenderingFeaturesKHR enabledDynamicRenderingFeaturesKHR {};

    class Appender {
    public:
        Appender()
            : m_pNext { nullptr }
        {
        }

        template <typename T>
        void AppendNext(T* newNext, VkStructureType structureType)
        {
            newNext->sType = structureType;
            newNext->pNext = m_pNext;
            m_pNext = newNext;
        }

        template <typename T>
        void AppendNext(T* newNext)
        {
            newNext->pNext = m_pNext;
            m_pNext = newNext;
        }

        void* GetNext() const
        {
            return m_pNext;
        }

        void Clear()
        {
            m_pNext = nullptr;
        }

    protected:
        void* m_pNext;
    };

    Appender featuresAppender;
    // Appender propertiesAppender;
};

}