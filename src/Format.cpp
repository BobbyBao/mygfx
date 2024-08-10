#include "Format.h"

#include <tiny_imageformat/tinyimageformat.h>

namespace mygfx {

struct FormatSizeHelper {
    FormatInfo formatSize[(int)Format::COUNT];

    FormatSizeHelper()
    {
        for (int i = 0; i < (int)Format::COUNT; i++) {
            auto fmt = (Format)i;
            formatSize[i] = FormatInfo {
                .blockSizeInBits = getBitSizeOfBlock(fmt),
                .blockWidth = (uint8_t)getWidthOfBlock(fmt),
                .blockHeight = (uint8_t)getHeightOfBlock(fmt),
                .blockDepth = (uint8_t)getDepthOfBlock(fmt),
                .compressed = TinyImageFormat_IsCompressed((TinyImageFormat)fmt),
                .depth = TinyImageFormat_IsDepthOnly((TinyImageFormat)fmt) || TinyImageFormat_IsDepthAndStencil((TinyImageFormat)fmt),
                .stencil = TinyImageFormat_IsStencilOnly((TinyImageFormat)fmt) || TinyImageFormat_IsDepthAndStencil((TinyImageFormat)fmt),
            };
        }
    }
};

static FormatSizeHelper helper;

uint32_t getWidthOfBlock(Format const fmt)
{
    return TinyImageFormat_WidthOfBlock((TinyImageFormat)fmt);
}

uint32_t getHeightOfBlock(Format const fmt)
{
    return TinyImageFormat_HeightOfBlock((TinyImageFormat)fmt);
}

uint32_t getDepthOfBlock(Format const fmt)
{
    return TinyImageFormat_DepthOfBlock((TinyImageFormat)fmt);
}

uint32_t getBitSizeOfBlock(Format const fmt)
{
    return TinyImageFormat_BitSizeOfBlock((TinyImageFormat)fmt);
}

uint32_t getChannelCount(Format const fmt)
{
    return TinyImageFormat_ChannelCount((TinyImageFormat)fmt);
}

const FormatInfo& getFormatInfo(Format const fmt)
{
    return helper.formatSize[(int)fmt];
}

char const* const getName(Format const fmt)
{
    return TinyImageFormat_Name((TinyImageFormat)fmt);
}

Format fromName(char const* p)
{
    return (Format)TinyImageFormat_FromName(p);
}

Format convertIntoGammaFormat(Format format)
{
    switch (format) {
    default:
        break;
    case Format::R8_UNORM:
        return Format::R8_SRGB;
    case Format::R8G8_UNORM:
        return Format::R8G8_SRGB;
    case Format::R8G8B8_UNORM:
        return Format::R8G8B8_SRGB;
    case Format::B8G8R8_UNORM:
        return Format::B8G8R8_SRGB;
    case Format::R8G8B8A8_UNORM:
        return Format::R8G8B8A8_SRGB;
    case Format::B8G8R8A8_UNORM:
        return Format::B8G8R8A8_SRGB;
    // case Format::A8B8G8R8_UNORM:
    //     return Format::A8B8G8R8_SRGB;
    case Format::DXBC1_RGB_UNORM:
        return Format::DXBC1_RGB_SRGB;
    case Format::DXBC1_RGBA_UNORM:
        return Format::DXBC1_RGBA_SRGB;
    case Format::DXBC2_UNORM:
        return Format::DXBC2_SRGB;
    case Format::DXBC3_UNORM:
        return Format::DXBC3_SRGB;
    case Format::DXBC7_UNORM:
        return Format::DXBC7_SRGB;
    case Format::ETC2_R8G8B8_UNORM:
        return Format::ETC2_R8G8B8_SRGB;
    case Format::ETC2_R8G8B8A1_UNORM:
        return Format::ETC2_R8G8B8A1_SRGB;
    case Format::ETC2_R8G8B8A8_UNORM:
        return Format::ETC2_R8G8B8A8_SRGB;
    case Format::ASTC_4x4_UNORM:
        return Format::ASTC_4x4_SRGB;
    case Format::ASTC_5x4_UNORM:
        return Format::ASTC_5x4_SRGB;
    case Format::ASTC_5x5_UNORM:
        return Format::ASTC_5x5_SRGB;
    case Format::ASTC_6x5_UNORM:
        return Format::ASTC_6x5_SRGB;
    case Format::ASTC_6x6_UNORM:
        return Format::ASTC_6x6_SRGB;
    case Format::ASTC_8x5_UNORM:
        return Format::ASTC_8x5_SRGB;
    case Format::ASTC_8x6_UNORM:
        return Format::ASTC_8x6_SRGB;
    case Format::ASTC_8x8_UNORM:
        return Format::ASTC_8x8_SRGB;
    case Format::ASTC_10x5_UNORM:
        return Format::ASTC_10x5_SRGB;
    case Format::ASTC_10x6_UNORM:
        return Format::ASTC_10x6_SRGB;
    case Format::ASTC_10x8_UNORM:
        return Format::ASTC_10x8_SRGB;
    case Format::ASTC_10x10_UNORM:
        return Format::ASTC_10x10_SRGB;
    case Format::ASTC_12x10_UNORM:
        return Format::ASTC_12x10_SRGB;
    case Format::ASTC_12x12_UNORM:
        return Format::ASTC_12x12_SRGB;
    }

    return format;
}

Format convertIntoNonGammaFormat(Format format)
{
    switch (format) {
    default:
        break;
    case Format::R8_SRGB:
        return Format::R8_UNORM;
    case Format::R8G8_SRGB:
        return Format::R8G8_UNORM;
    case Format::R8G8B8_SRGB:
        return Format::R8G8B8_UNORM;
    case Format::B8G8R8_SRGB:
        return Format::B8G8R8_UNORM;
    case Format::R8G8B8A8_SRGB:
        return Format::R8G8B8A8_UNORM;
    case Format::B8G8R8A8_SRGB:
        return Format::B8G8R8A8_UNORM;
    // case Format::A8B8G8R8_SRGB:
    //     return Format::A8B8G8R8_UNORM;
    case Format::DXBC1_RGB_SRGB:
        return Format::DXBC1_RGB_UNORM;
    case Format::DXBC1_RGBA_SRGB:
        return Format::DXBC1_RGBA_UNORM;
    case Format::DXBC2_SRGB:
        return Format::DXBC2_UNORM;
    case Format::DXBC3_SRGB:
        return Format::DXBC3_UNORM;
    case Format::DXBC7_SRGB:
        return Format::DXBC7_UNORM;
    case Format::ETC2_R8G8B8_SRGB:
        return Format::ETC2_R8G8B8_UNORM;
    case Format::ETC2_R8G8B8A1_SRGB:
        return Format::ETC2_R8G8B8A1_UNORM;
    case Format::ETC2_R8G8B8A8_SRGB:
        return Format::ETC2_R8G8B8A8_UNORM;
    case Format::ASTC_4x4_SRGB:
        return Format::ASTC_4x4_UNORM;
    case Format::ASTC_5x4_SRGB:
        return Format::ASTC_5x4_UNORM;
    case Format::ASTC_5x5_SRGB:
        return Format::ASTC_5x5_UNORM;
    case Format::ASTC_6x5_SRGB:
        return Format::ASTC_6x5_UNORM;
    case Format::ASTC_6x6_SRGB:
        return Format::ASTC_6x6_UNORM;
    case Format::ASTC_8x5_SRGB:
        return Format::ASTC_8x5_UNORM;
    case Format::ASTC_8x6_SRGB:
        return Format::ASTC_8x6_UNORM;
    case Format::ASTC_8x8_SRGB:
        return Format::ASTC_8x8_UNORM;
    case Format::ASTC_10x5_SRGB:
        return Format::ASTC_10x5_UNORM;
    case Format::ASTC_10x6_SRGB:
        return Format::ASTC_10x6_UNORM;
    case Format::ASTC_10x8_SRGB:
        return Format::ASTC_10x8_UNORM;
    case Format::ASTC_10x10_SRGB:
        return Format::ASTC_10x10_UNORM;
    case Format::ASTC_12x10_SRGB:
        return Format::ASTC_12x10_UNORM;
    case Format::ASTC_12x12_SRGB:
        return Format::ASTC_12x12_UNORM;
    }

    return format;
}

Format setFormatGamma(Format format, bool addGamma)
{
    if (addGamma) {
        return convertIntoGammaFormat(format);
    } else {
        return convertIntoNonGammaFormat(format);
    }
}

}