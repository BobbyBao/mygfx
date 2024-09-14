#include "TextureLoader.h"
#include "GraphicsApi.h"
#include "Texture.h"
#include "utils/FileUtils.h"
#include <ktx.h>
#include <gl_format.h>
#include <tiny_imageformat/tinyimageformat_apis.h>

namespace mygfx {

static inline Format getFormatFromOpenGLInternalFormat(const GLenum internalFormat)
{
    switch (internalFormat) {
        //
        // 8 bits per component
        //
    case GL_R8:
        return Format::R8_UNORM; // 1-component, 8-bit unsigned normalized
    case GL_RG8:
        return Format::R8G8_UNORM; // 2-component, 8-bit unsigned normalized
    case GL_RGB8:
        return Format::R8G8B8_UNORM; // 3-component, 8-bit unsigned normalized
    case GL_RGBA8:
        return Format::R8G8B8A8_UNORM; // 4-component, 8-bit unsigned normalized

    case GL_R8_SNORM:
        return Format::R8_SNORM; // 1-component, 8-bit signed normalized
    case GL_RG8_SNORM:
        return Format::R8G8_SNORM; // 2-component, 8-bit signed normalized
    case GL_RGB8_SNORM:
        return Format::R8G8B8_SNORM; // 3-component, 8-bit signed normalized
    case GL_RGBA8_SNORM:
        return Format::R8G8B8A8_SNORM; // 4-component, 8-bit signed normalized

    case GL_R8UI:
        return Format::R8_UINT; // 1-component, 8-bit unsigned integer
    case GL_RG8UI:
        return Format::R8G8_UINT; // 2-component, 8-bit unsigned integer
    case GL_RGB8UI:
        return Format::R8G8B8_UINT; // 3-component, 8-bit unsigned integer
    case GL_RGBA8UI:
        return Format::R8G8B8A8_UINT; // 4-component, 8-bit unsigned integer

    case GL_R8I:
        return Format::R8_SINT; // 1-component, 8-bit signed integer
    case GL_RG8I:
        return Format::R8G8_SINT; // 2-component, 8-bit signed integer
    case GL_RGB8I:
        return Format::R8G8B8_SINT; // 3-component, 8-bit signed integer
    case GL_RGBA8I:
        return Format::R8G8B8A8_SINT; // 4-component, 8-bit signed integer

    case GL_SR8:
        return Format::R8_SRGB; // 1-component, 8-bit sRGB
    case GL_SRG8:
        return Format::R8G8_SRGB; // 2-component, 8-bit sRGB
    case GL_SRGB8:
        return Format::R8G8B8_SRGB; // 3-component, 8-bit sRGB
    case GL_SRGB8_ALPHA8:
        return Format::R8G8B8A8_SRGB; // 4-component, 8-bit sRGB

        //
        // 16 bits per component
        //
    case GL_R16:
        return Format::R16_UNORM; // 1-component, 16-bit unsigned normalized
    case GL_RG16:
        return Format::R16G16_UNORM; // 2-component, 16-bit unsigned normalized
    case GL_RGB16:
        return Format::R16G16B16_UNORM; // 3-component, 16-bit unsigned normalized
    case GL_RGBA16:
        return Format::R16G16B16A16_UNORM; // 4-component, 16-bit unsigned normalized

    case GL_R16_SNORM:
        return Format::R16_SNORM; // 1-component, 16-bit signed normalized
    case GL_RG16_SNORM:
        return Format::R16G16_SNORM; // 2-component, 16-bit signed normalized
    case GL_RGB16_SNORM:
        return Format::R16G16B16_SNORM; // 3-component, 16-bit signed normalized
    case GL_RGBA16_SNORM:
        return Format::R16G16B16A16_SNORM; // 4-component, 16-bit signed normalized

    case GL_R16UI:
        return Format::R16_UINT; // 1-component, 16-bit unsigned integer
    case GL_RG16UI:
        return Format::R16G16_UINT; // 2-component, 16-bit unsigned integer
    case GL_RGB16UI:
        return Format::R16G16B16_UINT; // 3-component, 16-bit unsigned integer
    case GL_RGBA16UI:
        return Format::R16G16B16A16_UINT; // 4-component, 16-bit unsigned integer

    case GL_R16I:
        return Format::R16_SINT; // 1-component, 16-bit signed integer
    case GL_RG16I:
        return Format::R16G16_SINT; // 2-component, 16-bit signed integer
    case GL_RGB16I:
        return Format::R16G16B16_SINT; // 3-component, 16-bit signed integer
    case GL_RGBA16I:
        return Format::R16G16B16A16_SINT; // 4-component, 16-bit signed integer

    case GL_R16F:
        return Format::R16_SFLOAT; // 1-component, 16-bit floating-point
    case GL_RG16F:
        return Format::R16G16_SFLOAT; // 2-component, 16-bit floating-point
    case GL_RGB16F:
        return Format::R16G16B16_SFLOAT; // 3-component, 16-bit floating-point
    case GL_RGBA16F:
        return Format::R16G16B16A16_SFLOAT; // 4-component, 16-bit floating-point

        //
        // 32 bits per component
        //
    case GL_R32UI:
        return Format::R32_UINT; // 1-component, 32-bit unsigned integer
    case GL_RG32UI:
        return Format::R32G32_UINT; // 2-component, 32-bit unsigned integer
    case GL_RGB32UI:
        return Format::R32G32B32_UINT; // 3-component, 32-bit unsigned integer
    case GL_RGBA32UI:
        return Format::R32G32B32A32_UINT; // 4-component, 32-bit unsigned integer

    case GL_R32I:
        return Format::R32_SINT; // 1-component, 32-bit signed integer
    case GL_RG32I:
        return Format::R32G32_SINT; // 2-component, 32-bit signed integer
    case GL_RGB32I:
        return Format::R32G32B32_SINT; // 3-component, 32-bit signed integer
    case GL_RGBA32I:
        return Format::R32G32B32A32_SINT; // 4-component, 32-bit signed integer

    case GL_R32F:
        return Format::R32_SFLOAT; // 1-component, 32-bit floating-point
    case GL_RG32F:
        return Format::R32G32_SFLOAT; // 2-component, 32-bit floating-point
    case GL_RGB32F:
        return Format::R32G32B32_SFLOAT; // 3-component, 32-bit floating-point
    case GL_RGBA32F:
        return Format::R32G32B32A32_SFLOAT; // 4-component, 32-bit floating-point

        //
        // Packed
        //
    case GL_R3_G3_B2:
        return Format::UNDEFINED; // 3-component 3:3:2,       unsigned normalized
    case GL_RGB4:
        return Format::UNDEFINED; // 3-component 4:4:4,       unsigned normalized
    case GL_RGB5:
        return Format::R5G5B5A1_UNORM; // 3-component 5:5:5,       unsigned normalized
    case GL_RGB565:
        return Format::R5G6B5_UNORM; // 3-component 5:6:5,       unsigned normalized
    case GL_RGB10:
        return Format::A2R10G10B10_UNORM; // 3-component 10:10:10,    unsigned normalized
    case GL_RGB12:
        return Format::UNDEFINED; // 3-component 12:12:12,    unsigned normalized
    case GL_RGBA2:
        return Format::UNDEFINED; // 4-component 2:2:2:2,     unsigned normalized
    case GL_RGBA4:
        return Format::R4G4B4A4_UNORM; // 4-component 4:4:4:4,     unsigned normalized
    case GL_RGBA12:
        return Format::UNDEFINED; // 4-component 12:12:12:12, unsigned normalized
    case GL_RGB5_A1:
        return Format::A1R5G5B5_UNORM; // 4-component 5:5:5:1,     unsigned normalized
    case GL_RGB10_A2:
        return Format::A2R10G10B10_UNORM; // 4-component 10:10:10:2,  unsigned normalized
    case GL_RGB10_A2UI:
        return Format::A2R10G10B10_UINT; // 4-component 10:10:10:2,  unsigned integer
    case GL_R11F_G11F_B10F:
        return Format::B10G11R11_UFLOAT; // 3-component 11:11:10,    floating-point
    case GL_RGB9_E5:
        return Format::E5B9G9R9_UFLOAT; // 3-component/exp 9:9:9/5, floating-point

        //
        // S3TC/DXT/BC
        //

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        return Format::DXBC1_RGB_UNORM; // line through 3D space, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        return Format::DXBC1_RGBA_UNORM; // line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        return Format::DXBC2_UNORM; // line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        return Format::DXBC3_UNORM; // line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

    case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        return Format::DXBC1_RGB_SRGB; // line through 3D space, 4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        return Format::DXBC1_RGBA_SRGB; // line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        return Format::DXBC2_SRGB; // line through 3D space plus line through 1D space, 4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        return Format::DXBC3_SRGB; // line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

    case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        return Format::DXBC4_UNORM; // line through 1D space, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        return Format::DXBC5_UNORM; // two lines through 1D space, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        return Format::DXBC4_SNORM; // line through 1D space, 4x4 blocks, signed normalized
    case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        return Format::DXBC5_SNORM; // two lines through 1D space, 4x4 blocks, signed normalized

    case GL_COMPRESSED_RED_RGTC1:
        return Format::DXBC4_UNORM; // line through 1D space, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RG_RGTC2:
        return Format::DXBC5_UNORM; // two lines through 1D space, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_SIGNED_RED_RGTC1:
        return Format::DXBC4_SNORM; // line through 1D space, 4x4 blocks, signed normalized
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
        return Format::DXBC5_SNORM; // two lines through 1D space, 4x4 blocks, signed normalized

    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        return Format::DXBC6H_UFLOAT; // 3-component, 4x4 blocks, unsigned floating-point
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        return Format::DXBC6H_SFLOAT; // 3-component, 4x4 blocks, signed floating-point
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
        return Format::DXBC7_UNORM; // 4-component, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        return Format::DXBC7_SRGB; // 4-component, 4x4 blocks, sRGB

        //
        // ETC
        //
    case GL_ETC1_RGB8_OES:
        return Format::ETC2_R8G8B8_UNORM; // 3-component ETC1, 4x4 blocks, unsigned normalized

    case GL_COMPRESSED_RGB8_ETC2:
        return Format::ETC2_R8G8B8_UNORM; // 3-component ETC2, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        return Format::ETC2_R8G8B8A1_UNORM; // 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA8_ETC2_EAC:
        return Format::ETC2_R8G8B8A8_UNORM; // 4-component ETC2, 4x4 blocks, unsigned normalized

    case GL_COMPRESSED_SRGB8_ETC2:
        return Format::ETC2_R8G8B8_SRGB; // 3-component ETC2, 4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        return Format::ETC2_R8G8B8A1_SRGB; // 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        return Format::ETC2_R8G8B8A8_SRGB; // 4-component ETC2, 4x4 blocks, sRGB
#if false
		case GL_COMPRESSED_R11_EAC:								return Format::EAC_R11_UNORM;		// 1-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RG11_EAC:							return Format::EAC_R11G11_UNORM;	// 2-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_R11_EAC:						return Format::EAC_R11_SNORM;		// 1-component ETC, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_RG11_EAC:						return Format::EAC_R11G11_SNORM;	// 2-component ETC, 4x4 blocks, signed normalized
#endif
        //
        // PVRTC
        //
    case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        return Format::PVRTC1_2BPP_UNORM; // 3-component PVRTC, 16x8 blocks, unsigned normalized
    case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
        return Format::PVRTC1_4BPP_UNORM; // 3-component PVRTC,  8x8 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
        return Format::PVRTC1_2BPP_UNORM; // 4-component PVRTC, 16x8 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
        return Format::PVRTC1_4BPP_UNORM; // 4-component PVRTC,  8x8 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG:
        return Format::PVRTC2_2BPP_UNORM; // 4-component PVRTC,  8x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG:
        return Format::PVRTC2_4BPP_UNORM; // 4-component PVRTC,  4x4 blocks, unsigned normalized

    case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT:
        return Format::PVRTC1_2BPP_SRGB; // 3-component PVRTC, 16x8 blocks, sRGB
    case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT:
        return Format::PVRTC1_4BPP_SRGB; // 3-component PVRTC,  8x8 blocks, sRGB
    case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT:
        return Format::PVRTC1_2BPP_SRGB; // 4-component PVRTC, 16x8 blocks, sRGB
    case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT:
        return Format::PVRTC1_4BPP_SRGB; // 4-component PVRTC,  8x8 blocks, sRGB
    case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG:
        return Format::PVRTC2_2BPP_SRGB; // 4-component PVRTC,  8x4 blocks, sRGB
    case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:
        return Format::PVRTC2_4BPP_SRGB; // 4-component PVRTC,  4x4 blocks, sRGB

        //
        // ASTC
        //
    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
        return Format::ASTC_4x4_UNORM; // 4-component ASTC, 4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
        return Format::ASTC_5x4_UNORM; // 4-component ASTC, 5x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
        return Format::ASTC_5x5_UNORM; // 4-component ASTC, 5x5 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
        return Format::ASTC_6x5_UNORM; // 4-component ASTC, 6x5 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
        return Format::ASTC_6x6_UNORM; // 4-component ASTC, 6x6 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
        return Format::ASTC_8x5_UNORM; // 4-component ASTC, 8x5 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
        return Format::ASTC_8x6_UNORM; // 4-component ASTC, 8x6 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
        return Format::ASTC_8x8_UNORM; // 4-component ASTC, 8x8 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
        return Format::ASTC_10x5_UNORM; // 4-component ASTC, 10x5 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
        return Format::ASTC_10x6_UNORM; // 4-component ASTC, 10x6 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
        return Format::ASTC_10x8_UNORM; // 4-component ASTC, 10x8 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
        return Format::ASTC_10x10_UNORM; // 4-component ASTC, 10x10 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
        return Format::ASTC_12x10_UNORM; // 4-component ASTC, 12x10 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
        return Format::ASTC_12x12_UNORM; // 4-component ASTC, 12x12 blocks, unsigned normalized

    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
        return Format::ASTC_4x4_SRGB; // 4-component ASTC, 4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
        return Format::ASTC_5x4_SRGB; // 4-component ASTC, 5x4 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
        return Format::ASTC_5x5_SRGB; // 4-component ASTC, 5x5 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
        return Format::ASTC_6x5_SRGB; // 4-component ASTC, 6x5 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
        return Format::ASTC_6x6_SRGB; // 4-component ASTC, 6x6 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
        return Format::ASTC_8x5_SRGB; // 4-component ASTC, 8x5 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
        return Format::ASTC_8x6_SRGB; // 4-component ASTC, 8x6 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
        return Format::ASTC_8x8_SRGB; // 4-component ASTC, 8x8 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
        return Format::ASTC_10x5_SRGB; // 4-component ASTC, 10x5 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
        return Format::ASTC_10x6_SRGB; // 4-component ASTC, 10x6 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
        return Format::ASTC_10x8_SRGB; // 4-component ASTC, 10x8 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
        return Format::ASTC_10x10_SRGB; // 4-component ASTC, 10x10 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
        return Format::ASTC_12x10_SRGB; // 4-component ASTC, 12x10 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
        return Format::ASTC_12x12_SRGB; // 4-component ASTC, 12x12 blocks, sRGB

    case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:
        return Format::UNDEFINED; // 4-component ASTC, 3x3x3 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:
        return Format::UNDEFINED; // 4-component ASTC, 4x3x3 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:
        return Format::UNDEFINED; // 4-component ASTC, 4x4x3 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:
        return Format::UNDEFINED; // 4-component ASTC, 4x4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:
        return Format::UNDEFINED; // 4-component ASTC, 5x4x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:
        return Format::UNDEFINED; // 4-component ASTC, 5x5x4 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:
        return Format::UNDEFINED; // 4-component ASTC, 5x5x5 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:
        return Format::UNDEFINED; // 4-component ASTC, 6x5x5 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:
        return Format::UNDEFINED; // 4-component ASTC, 6x6x5 blocks, unsigned normalized
    case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:
        return Format::UNDEFINED; // 4-component ASTC, 6x6x6 blocks, unsigned normalized

    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:
        return Format::UNDEFINED; // 4-component ASTC, 3x3x3 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:
        return Format::UNDEFINED; // 4-component ASTC, 4x3x3 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:
        return Format::UNDEFINED; // 4-component ASTC, 4x4x3 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:
        return Format::UNDEFINED; // 4-component ASTC, 4x4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:
        return Format::UNDEFINED; // 4-component ASTC, 5x4x4 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:
        return Format::UNDEFINED; // 4-component ASTC, 5x5x4 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:
        return Format::UNDEFINED; // 4-component ASTC, 5x5x5 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:
        return Format::UNDEFINED; // 4-component ASTC, 6x5x5 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:
        return Format::UNDEFINED; // 4-component ASTC, 6x6x5 blocks, sRGB
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:
        return Format::UNDEFINED; // 4-component ASTC, 6x6x6 blocks, sRGB

        //
        // ATC
        //
    case GL_ATC_RGB_AMD:
        return Format::UNDEFINED; // 3-component, 4x4 blocks, unsigned normalized
    case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:
        return Format::UNDEFINED; // 4-component, 4x4 blocks, unsigned normalized
    case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:
        return Format::UNDEFINED; // 4-component, 4x4 blocks, unsigned normalized

        //
        // Palletized
        //
    case GL_PALETTE4_RGB8_OES:
        return Format::UNDEFINED; // 3-component 8:8:8,   4-bit palette, unsigned normalized
    case GL_PALETTE4_RGBA8_OES:
        return Format::UNDEFINED; // 4-component 8:8:8:8, 4-bit palette, unsigned normalized
    case GL_PALETTE4_R5_G6_B5_OES:
        return Format::UNDEFINED; // 3-component 5:6:5,   4-bit palette, unsigned normalized
    case GL_PALETTE4_RGBA4_OES:
        return Format::UNDEFINED; // 4-component 4:4:4:4, 4-bit palette, unsigned normalized
    case GL_PALETTE4_RGB5_A1_OES:
        return Format::UNDEFINED; // 4-component 5:5:5:1, 4-bit palette, unsigned normalized
    case GL_PALETTE8_RGB8_OES:
        return Format::UNDEFINED; // 3-component 8:8:8,   8-bit palette, unsigned normalized
    case GL_PALETTE8_RGBA8_OES:
        return Format::UNDEFINED; // 4-component 8:8:8:8, 8-bit palette, unsigned normalized
    case GL_PALETTE8_R5_G6_B5_OES:
        return Format::UNDEFINED; // 3-component 5:6:5,   8-bit palette, unsigned normalized
    case GL_PALETTE8_RGBA4_OES:
        return Format::UNDEFINED; // 4-component 4:4:4:4, 8-bit palette, unsigned normalized
    case GL_PALETTE8_RGB5_A1_OES:
        return Format::UNDEFINED; // 4-component 5:5:5:1, 8-bit palette, unsigned normalized

        //
        // Depth/stencil
        //
    case GL_DEPTH_COMPONENT16:
        return Format::D16_UNORM;
    case GL_DEPTH_COMPONENT24:
        return Format::X8_D24_UNORM;
    case GL_DEPTH_COMPONENT32:
        return Format::UNDEFINED;
    case GL_DEPTH_COMPONENT32F:
        return Format::D32_SFLOAT;
    case GL_DEPTH_COMPONENT32F_NV:
        return Format::D32_SFLOAT;
    case GL_STENCIL_INDEX1:
        return Format::UNDEFINED;
    case GL_STENCIL_INDEX4:
        return Format::UNDEFINED;
    case GL_STENCIL_INDEX8:
        return Format::S8_UINT;
    case GL_STENCIL_INDEX16:
        return Format::UNDEFINED;
    case GL_DEPTH24_STENCIL8:
        return Format::D24_UNORM_S8_UINT;
    case GL_DEPTH32F_STENCIL8:
        return Format::D32_SFLOAT_S8_UINT;
    case GL_DEPTH32F_STENCIL8_NV:
        return Format::D32_SFLOAT_S8_UINT;

    default:
        return Format::UNDEFINED;
    }
}

Ref<Texture> TextureLoader::load(const String& fileName, SamplerInfo samplerInfo)
{
    auto content = FileUtils::readAll(fileName);
    return onLoad(Span<uint8_t> { content }, samplerInfo);
}

KtxTextureLoader::~KtxTextureLoader()
{
}

Ref<Texture> KtxTextureLoader::onLoad(const Span<uint8_t>& content, SamplerInfo samplerInfo)
{
    ktxTexture* pKtxTexture = nullptr;
    auto result = ktxTexture_CreateFromMemory(content.data(), content.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &pKtxTexture);
    assert(result == KTX_SUCCESS);
    TextureData textureData;
    textureData.width = pKtxTexture->baseWidth;
    textureData.height = pKtxTexture->baseHeight;
    textureData.depth = pKtxTexture->baseDepth;
    textureData.mipMapCount = pKtxTexture->numLevels;
    textureData.layerCount = pKtxTexture->numLayers;
    textureData.faceCount = pKtxTexture->numFaces;

    textureData.format = getFormatFromOpenGLInternalFormat(pKtxTexture->glInternalformat);
    if (samplerInfo.srgb) {
        textureData.format = setFormatGamma(textureData.format, true);
    }

    ktxTextureData = ktxTexture_GetData(pKtxTexture);
    ktxTextureSize = ktxTexture_GetSize(pKtxTexture);
    mKtxTexture = pKtxTexture;
    Ref<Texture> tex(new Texture());
    if (!tex->create(textureData, samplerInfo)) {
        return nullptr;
    }

    tex->copyData(this);
    return tex;
}

void KtxTextureLoader::copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level)
{
    ktx_size_t offset;
    KTX_error_code result = ktxTexture_GetImageOffset((ktxTexture*)mKtxTexture, level, layer, face, &offset);
    assert(result == KTX_SUCCESS);
    std::memcpy(pDest, ktxTextureData + offset, imageSize);
}

struct DDS_PIXELFORMAT {
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t bitCount;
    uint32_t bitMaskR;
    uint32_t bitMaskG;
    uint32_t bitMaskB;
    uint32_t bitMaskA;
};

struct DDS_HEADER {

    uint32_t dwSize;
    uint32_t dwHeaderFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t dwSurfaceFlags;
    uint32_t dwCubemapFlags;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

//--------------------------------------------------------------------------------------
// retrieve the GetDxgiFormat from a DDS_PIXELFORMAT
//--------------------------------------------------------------------------------------
static TinyImageFormat_DXGI_FORMAT GetDxgiFormat(DDS_PIXELFORMAT pixelFmt)
{
    if (pixelFmt.flags & 0x00000004) // DDPF_FOURCC
    {
        // Check for D3DFORMAT enums being set here
        switch (pixelFmt.fourCC) {
        case '1TXD':
            return TIF_DXGI_FORMAT_BC1_UNORM;
        case '3TXD':
            return TIF_DXGI_FORMAT_BC2_UNORM;
        case '5TXD':
            return TIF_DXGI_FORMAT_BC3_UNORM;
        case 'U4CB':
            return TIF_DXGI_FORMAT_BC4_UNORM;
        case 'A4CB':
            return TIF_DXGI_FORMAT_BC4_SNORM;
        case '2ITA':
            return TIF_DXGI_FORMAT_BC5_UNORM;
        case 'S5CB':
            return TIF_DXGI_FORMAT_BC5_SNORM;
        case 'GBGR':
            return TIF_DXGI_FORMAT_R8G8_B8G8_UNORM;
        case 'BGRG':
            return TIF_DXGI_FORMAT_G8R8_G8B8_UNORM;
        case 36:
            return TIF_DXGI_FORMAT_R16G16B16A16_UNORM;
        case 110:
            return TIF_DXGI_FORMAT_R16G16B16A16_SNORM;
        case 111:
            return TIF_DXGI_FORMAT_R16_FLOAT;
        case 112:
            return TIF_DXGI_FORMAT_R16G16_FLOAT;
        case 113:
            return TIF_DXGI_FORMAT_R16G16B16A16_FLOAT;
        case 114:
            return TIF_DXGI_FORMAT_R32_FLOAT;
        case 115:
            return TIF_DXGI_FORMAT_R32G32_FLOAT;
        case 116:
            return TIF_DXGI_FORMAT_R32G32B32A32_FLOAT;
        default:
            return TIF_DXGI_FORMAT_UNKNOWN;
        }
    } else {
        switch (pixelFmt.bitMaskR) {
        case 0xff:
            return TIF_DXGI_FORMAT_R8G8B8A8_UNORM;
        case 0x00ff0000:
            return TIF_DXGI_FORMAT_B8G8R8A8_UNORM;
        case 0xffff:
            return TIF_DXGI_FORMAT_R16G16_UNORM;
        case 0x3ff:
            return TIF_DXGI_FORMAT_R10G10B10A2_UNORM;
        case 0x7c00:
            return TIF_DXGI_FORMAT_B5G5R5A1_UNORM;
        case 0xf800:
            return TIF_DXGI_FORMAT_B5G6R5_UNORM;
        case 0:
            return TIF_DXGI_FORMAT_A8_UNORM;
        default:
            return TIF_DXGI_FORMAT_UNKNOWN;
        };
    }
}

static TinyImageFormat_DXGI_FORMAT GetSrgbFormat(TinyImageFormat_DXGI_FORMAT pixelFmt)
{
    switch (pixelFmt) {
    case TIF_DXGI_FORMAT_BC1_UNORM:
        return TIF_DXGI_FORMAT_BC1_UNORM_SRGB;
    case TIF_DXGI_FORMAT_BC2_UNORM:
        return TIF_DXGI_FORMAT_BC2_UNORM_SRGB;
    case TIF_DXGI_FORMAT_BC3_UNORM:
        return TIF_DXGI_FORMAT_BC3_UNORM_SRGB;
    case TIF_DXGI_FORMAT_BC7_UNORM:
        return TIF_DXGI_FORMAT_BC7_UNORM_SRGB;
    case TIF_DXGI_FORMAT_R8G8B8A8_UNORM:
        return TIF_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case TIF_DXGI_FORMAT_B8G8R8A8_UNORM:
        return TIF_DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    case TIF_DXGI_FORMAT_B8G8R8X8_UNORM:
        return TIF_DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
    default:
        return pixelFmt;
    }
}

DDSTextureLoader::~DDSTextureLoader()
{
}

Ref<Texture> DDSTextureLoader::onLoad(const Span<uint8_t>& content, SamplerInfo samplerInfo)
{
    typedef enum RESOURCE_DIMENSION {
        RESOURCE_DIMENSION_UNKNOWN = 0,
        RESOURCE_DIMENSION_BUFFER = 1,
        RESOURCE_DIMENSION_TEXTURE1D = 2,
        RESOURCE_DIMENSION_TEXTURE2D = 3,
        RESOURCE_DIMENSION_TEXTURE3D = 4
    } RESOURCE_DIMENSION;

    typedef struct
    {
        TinyImageFormat_DXGI_FORMAT dxgiFormat;
        RESOURCE_DIMENSION resourceDimension;
        uint32_t miscFlag;
        uint32_t arraySize;
        uint32_t reserved;
    } DDS_HEADER_DXT10;

    auto fileSize = content.size();
    auto rawTextureSize = fileSize;

    const int headerSize = 4 + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
    // read the header
    char headerData[headerSize];

    TextureData textureData;

    if (content.size() > headerSize) {
        std::memcpy(headerData, content.data(), headerSize);
        char* pByteData = headerData;
        uint32_t dwMagic = *reinterpret_cast<uint32_t*>(pByteData);
        if (dwMagic != ' SDD') // "DDS "
        {
            return nullptr;
        }

        pByteData += 4;
        rawTextureSize -= 4;

        DDS_HEADER* header = reinterpret_cast<DDS_HEADER*>(pByteData);
        pByteData += sizeof(DDS_HEADER);
        rawTextureSize -= sizeof(DDS_HEADER);

        textureData.width = header->dwWidth;
        textureData.height = header->dwHeight;
        textureData.depth = header->dwDepth ? header->dwDepth : 1;
        textureData.mipMapCount = header->dwMipMapCount ? header->dwMipMapCount : 1;

        if (header->ddspf.fourCC == '01XD') {
            DDS_HEADER_DXT10* header10 = reinterpret_cast<DDS_HEADER_DXT10*>((char*)header + sizeof(DDS_HEADER));
            rawTextureSize -= sizeof(DDS_HEADER_DXT10);

            textureData.layerCount = header10->arraySize;
            textureData.faceCount = 1;
            textureData.format = (Format)TinyImageFormat_FromDXGI_FORMAT(header10->dxgiFormat);
            assert(textureData.bitsPerPixel() == header->ddspf.bitCount);
        } else {

            auto dxgiFmt = GetDxgiFormat(header->ddspf);
            if (samplerInfo.srgb) {
                dxgiFmt = GetSrgbFormat(dxgiFmt);
            }

            textureData.layerCount = 1;
            textureData.faceCount = (header->dwCubemapFlags == 0xfe00) ? 6 : 1;
            textureData.format = (Format)TinyImageFormat_FromDXGI_FORMAT(dxgiFmt);
        }
    } else {
        assert(false);
        return nullptr;
    }

    mData = content.data() + fileSize - rawTextureSize;
    Ref<Texture> tex(new Texture());
    if (!tex->create(textureData, samplerInfo)) {
        return nullptr;
    }

    tex->copyData(this);
    return tex;
}

void DDSTextureLoader::copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level)
{
    std::memcpy(pDest, mData, imageSize);
    mData += imageSize;
}

}