#pragma once

#include "volk/volk.h"

#include <array>

#include "../utils/algorithm.h"
#include <vma/vk_mem_alloc.h>

#define VK_1_1 1
#define VK_1_2 2
#define VK_1_3 3

#define VK_VERSION 3

#define HAS_DYNAMIC_RENDERING

#if defined(VK_EXT_shader_object) && defined(HAS_DYNAMIC_RENDERING)
#define HAS_SHADER_OBJECT_EXT 0
#else
#define HAS_SHADER_OBJECT_EXT 0
#endif

namespace mygfx {

static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 16;
static constexpr uint32_t VARIABLE_DESC_COUNT = 4096;

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

}