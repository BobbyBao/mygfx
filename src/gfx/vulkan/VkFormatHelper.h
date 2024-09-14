#pragma once
#include "VulkanDefs.h"

#include <tiny_imageformat/tinyimageformat_apis.h>

namespace mygfx {

struct VkFormatInfo {
    uint32_t size;
    uint32_t channelCount;
    uint32_t pixelsPerBlock;

    VkFormatInfo(uint32_t size, uint32_t channelCount, uint32_t pixelsPerBlock)
    {
        this->size = size;
        this->channelCount = channelCount;
        this->pixelsPerBlock = pixelsPerBlock;
    }
};

uint32_t getBytesPerPixel(VkFormat format);
uint32_t bitsPerPixel(VkFormat fmt);
uint32_t getBytesPerBlock(VkFormat format);
uint32_t getPixelsPerBlock(VkFormat format);

VkFormat convertIntoGammaFormat(VkFormat format);
VkFormat convertIntoNonGammaFormat(VkFormat format);
VkFormat setFormatGamma(VkFormat format, bool addGamma);
bool isSrgb(VkFormat format);

inline bool isDepthOnlyFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT;
}

inline bool isDepthStencilFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT
        || format == VK_FORMAT_D32_SFLOAT_S8_UINT || isDepthOnlyFormat(format);
}

inline bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

bool IsCompressed_ETC2_EAC(VkFormat format);
bool IsCompressed_ASTC_LDR(VkFormat format);
bool IsCompressed_BC(VkFormat format);
bool IsCompressed_PVRTC(VkFormat format);
bool IsSinglePLANE422(VkFormat format);
bool IsCompressed(VkFormat format);

}