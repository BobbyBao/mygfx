#pragma once

#define USE_VOLK 1
#if USE_VOLK
#include "volk/volk.h"
#else
#include <vulkan/Vulkan.h>
#endif

#include <array>

#include "../utils/algorithm.h"
#include <vma/vk_mem_alloc.h>

#define MYGFX_FEATURE_LEVEL_0 0
#define MYGFX_FEATURE_LEVEL_1 1
#define MYGFX_FEATURE_LEVEL_2 2
#define MYGFX_FEATURE_LEVEL_3 3

#ifndef MYGFX_FEATURE_LEVEL

#if defined(VK_USE_PLATFORM_METAL_EXT)
#define MYGFX_FEATURE_LEVEL 2
#else
#define MYGFX_FEATURE_LEVEL 3
#endif

#endif

#define HAS_DYNAMIC_STATE1 1

#if defined(VK_EXT_shader_object) && (MYGFX_FEATURE_LEVEL == 3)
#define HAS_SHADER_OBJECT_EXT 1
#define HAS_DYNAMIC_STATE2 1
#define HAS_DYNAMIC_STATE3 1
#elif(MYGFX_FEATURE_LEVEL == 2)
#define HAS_SHADER_OBJECT_EXT 0
#define HAS_DYNAMIC_STATE2 1
#define HAS_DYNAMIC_STATE3 0
#elif(MYGFX_FEATURE_LEVEL == 1)
#define HAS_SHADER_OBJECT_EXT 0
#define HAS_DYNAMIC_STATE2 1
#define HAS_DYNAMIC_STATE3 0
#else
#define HAS_SHADER_OBJECT_EXT 0
#define HAS_DYNAMIC_STATE2 0
#define HAS_DYNAMIC_STATE3 0
#endif

namespace mygfx {

static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 16;
static constexpr uint32_t VARIABLE_DESC_COUNT = 4096;
static constexpr uint32_t IMAGE_VARIABLE_DESC_COUNT = 65536 / 2;
static constexpr uint32_t SAMPLER_VARIABLE_DESC_COUNT = 1024;

struct DescriptorResourceCounts : public std::array<uint32_t, DESCRIPTOR_TYPE_COUNT> {

    bool operator==(const DescriptorResourceCounts& other)
    {
        return std::memcmp(data(), other.data(), sizeof(uint32_t) * DESCRIPTOR_TYPE_COUNT) == 0;
    }
};

struct AttachmentFormats {
    uint32_t colorAttachmentCount = 0;
    VkFormat attachmentFormats[10] = { VK_FORMAT_UNDEFINED };
    VkFormat depthAttachmentFormat() const { return attachmentFormats[8]; }
    VkFormat stencilAttachmentFormat() const { return attachmentFormats[9]; }
    VkFormat& depthAttachmentFormat() { return attachmentFormats[8]; }
    VkFormat& stencilAttachmentFormat() { return attachmentFormats[9]; }
    size_t getHash() const { return mHash; }

    void reset()
    {
        std::memset(&attachmentFormats[0], 0, sizeof(VkFormat) * 10);
    }

    void calculateHash()
    {
        size_t hash = 0;
        for (uint32_t i = 0; i < colorAttachmentCount; i++) {
            utils::hash_combine(hash, attachmentFormats[i]);
        }

        utils::hash_combine(hash, attachmentFormats[8], attachmentFormats[9]);
        mHash = hash;
    }

private:
    size_t mHash = 0;
};

inline bool operator==(const VkDescriptorImageInfo& lhs, const VkDescriptorImageInfo& rhs) noexcept
{
    return std::memcmp(&lhs, &rhs, sizeof(VkDescriptorImageInfo)) == 0;
}

inline bool operator!=(const VkDescriptorImageInfo& lhs, const VkDescriptorImageInfo& rhs) noexcept
{
    return std::memcmp(&lhs, &rhs, sizeof(VkDescriptorImageInfo)) != 0;
}

#if !USE_VOLK

inline PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR { VK_NULL_HANDLE };

#define VK_FUNCTION(NAME) \
    inline PFN_##NAME NAME { VK_NULL_HANDLE }

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

#endif

}