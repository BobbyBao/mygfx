#include "VkFormatHelper.h"
// #include "Texture/DxgiFormatHelper.h"
#include <unordered_map>

namespace mygfx {

static VkFormatInfo formatUndefined(0, 0, 0);

static std::unordered_map<VkFormat, VkFormatInfo> vkFormatTable = {
    { VK_FORMAT_UNDEFINED, { 0, 0, 0 } },
    { VK_FORMAT_R4G4_UNORM_PACK8, { 1, 2, 1 } },
    { VK_FORMAT_R4G4B4A4_UNORM_PACK16, { 2, 4, 1 } },
    { VK_FORMAT_B4G4R4A4_UNORM_PACK16, { 2, 4, 1 } },
    { VK_FORMAT_R5G6B5_UNORM_PACK16, { 2, 3, 1 } },
    { VK_FORMAT_B5G6R5_UNORM_PACK16, { 2, 3, 1 } },
    { VK_FORMAT_R5G5B5A1_UNORM_PACK16, { 2, 4, 1 } },
    { VK_FORMAT_B5G5R5A1_UNORM_PACK16, { 2, 4, 1 } },
    { VK_FORMAT_A1R5G5B5_UNORM_PACK16, { 2, 4, 1 } },
    { VK_FORMAT_R8_UNORM, { 1, 1, 1 } },
    { VK_FORMAT_R8_SNORM, { 1, 1, 1 } },
    { VK_FORMAT_R8_USCALED, { 1, 1, 1 } },
    { VK_FORMAT_R8_SSCALED, { 1, 1, 1 } },
    { VK_FORMAT_R8_UINT, { 1, 1, 1 } },
    { VK_FORMAT_R8_SINT, { 1, 1, 1 } },
    { VK_FORMAT_R8_SRGB, { 1, 1, 1 } },
    { VK_FORMAT_R8G8_UNORM, { 2, 2, 1 } },
    { VK_FORMAT_R8G8_SNORM, { 2, 2, 1 } },
    { VK_FORMAT_R8G8_USCALED, { 2, 2, 1 } },
    { VK_FORMAT_R8G8_SSCALED, { 2, 2, 1 } },
    { VK_FORMAT_R8G8_UINT, { 2, 2, 1 } },
    { VK_FORMAT_R8G8_SINT, { 2, 2, 1 } },
    { VK_FORMAT_R8G8_SRGB, { 2, 2, 1 } },
    { VK_FORMAT_R8G8B8_UNORM, { 3, 3, 1 } },
    { VK_FORMAT_R8G8B8_SNORM, { 3, 3, 1 } },
    { VK_FORMAT_R8G8B8_USCALED, { 3, 3, 1 } },
    { VK_FORMAT_R8G8B8_SSCALED, { 3, 3, 1 } },
    { VK_FORMAT_R8G8B8_UINT, { 3, 3, 1 } },
    { VK_FORMAT_R8G8B8_SINT, { 3, 3, 1 } },
    { VK_FORMAT_R8G8B8_SRGB, { 3, 3, 1 } },
    { VK_FORMAT_B8G8R8_UNORM, { 3, 3, 1 } },
    { VK_FORMAT_B8G8R8_SNORM, { 3, 3, 1 } },
    { VK_FORMAT_B8G8R8_USCALED, { 3, 3, 1 } },
    { VK_FORMAT_B8G8R8_SSCALED, { 3, 3, 1 } },
    { VK_FORMAT_B8G8R8_UINT, { 3, 3, 1 } },
    { VK_FORMAT_B8G8R8_SINT, { 3, 3, 1 } },
    { VK_FORMAT_B8G8R8_SRGB, { 3, 3, 1 } },
    { VK_FORMAT_R8G8B8A8_UNORM, { 4, 4, 1 } },
    { VK_FORMAT_R8G8B8A8_SNORM, { 4, 4, 1 } },
    { VK_FORMAT_R8G8B8A8_USCALED, { 4, 4, 1 } },
    { VK_FORMAT_R8G8B8A8_SSCALED, { 4, 4, 1 } },
    { VK_FORMAT_R8G8B8A8_UINT, { 4, 4, 1 } },
    { VK_FORMAT_R8G8B8A8_SINT, { 4, 4, 1 } },
    { VK_FORMAT_R8G8B8A8_SRGB, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8A8_UNORM, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8A8_SNORM, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8A8_USCALED, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8A8_SSCALED, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8A8_UINT, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8A8_SINT, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8A8_SRGB, { 4, 4, 1 } },
    { VK_FORMAT_A8B8G8R8_UNORM_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A8B8G8R8_SNORM_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A8B8G8R8_USCALED_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A8B8G8R8_SSCALED_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A8B8G8R8_UINT_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A8B8G8R8_SINT_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A8B8G8R8_SRGB_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2R10G10B10_UNORM_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2R10G10B10_SNORM_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2R10G10B10_USCALED_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2R10G10B10_SSCALED_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2R10G10B10_UINT_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2R10G10B10_SINT_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2B10G10R10_UNORM_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2B10G10R10_SNORM_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2B10G10R10_USCALED_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2B10G10R10_SSCALED_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2B10G10R10_UINT_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_A2B10G10R10_SINT_PACK32, { 4, 4, 1 } },
    { VK_FORMAT_R16_UNORM, { 2, 1, 1 } },
    { VK_FORMAT_R16_SNORM, { 2, 1, 1 } },
    { VK_FORMAT_R16_USCALED, { 2, 1, 1 } },
    { VK_FORMAT_R16_SSCALED, { 2, 1, 1 } },
    { VK_FORMAT_R16_UINT, { 2, 1, 1 } },
    { VK_FORMAT_R16_SINT, { 2, 1, 1 } },
    { VK_FORMAT_R16_SFLOAT, { 2, 1, 1 } },
    { VK_FORMAT_R16G16_UNORM, { 4, 2, 1 } },
    { VK_FORMAT_R16G16_SNORM, { 4, 2, 1 } },
    { VK_FORMAT_R16G16_USCALED, { 4, 2, 1 } },
    { VK_FORMAT_R16G16_SSCALED, { 4, 2, 1 } },
    { VK_FORMAT_R16G16_UINT, { 4, 2, 1 } },
    { VK_FORMAT_R16G16_SINT, { 4, 2, 1 } },
    { VK_FORMAT_R16G16_SFLOAT, { 4, 2, 1 } },
    { VK_FORMAT_R16G16B16_UNORM, { 6, 3, 1 } },
    { VK_FORMAT_R16G16B16_SNORM, { 6, 3, 1 } },
    { VK_FORMAT_R16G16B16_USCALED, { 6, 3, 1 } },
    { VK_FORMAT_R16G16B16_SSCALED, { 6, 3, 1 } },
    { VK_FORMAT_R16G16B16_UINT, { 6, 3, 1 } },
    { VK_FORMAT_R16G16B16_SINT, { 6, 3, 1 } },
    { VK_FORMAT_R16G16B16_SFLOAT, { 6, 3, 1 } },
    { VK_FORMAT_R16G16B16A16_UNORM, { 8, 4, 1 } },
    { VK_FORMAT_R16G16B16A16_SNORM, { 8, 4, 1 } },
    { VK_FORMAT_R16G16B16A16_USCALED, { 8, 4, 1 } },
    { VK_FORMAT_R16G16B16A16_SSCALED, { 8, 4, 1 } },
    { VK_FORMAT_R16G16B16A16_UINT, { 8, 4, 1 } },
    { VK_FORMAT_R16G16B16A16_SINT, { 8, 4, 1 } },
    { VK_FORMAT_R16G16B16A16_SFLOAT, { 8, 4, 1 } },
    { VK_FORMAT_R32_UINT, { 4, 1, 1 } },
    { VK_FORMAT_R32_SINT, { 4, 1, 1 } },
    { VK_FORMAT_R32_SFLOAT, { 4, 1, 1 } },
    { VK_FORMAT_R32G32_UINT, { 8, 2, 1 } },
    { VK_FORMAT_R32G32_SINT, { 8, 2, 1 } },
    { VK_FORMAT_R32G32_SFLOAT, { 8, 2, 1 } },
    { VK_FORMAT_R32G32B32_UINT, { 12, 3, 1 } },
    { VK_FORMAT_R32G32B32_SINT, { 12, 3, 1 } },
    { VK_FORMAT_R32G32B32_SFLOAT, { 12, 3, 1 } },
    { VK_FORMAT_R32G32B32A32_UINT, { 16, 4, 1 } },
    { VK_FORMAT_R32G32B32A32_SINT, { 16, 4, 1 } },
    { VK_FORMAT_R32G32B32A32_SFLOAT, { 16, 4, 1 } },
    { VK_FORMAT_R64_UINT, { 8, 1, 1 } },
    { VK_FORMAT_R64_SINT, { 8, 1, 1 } },
    { VK_FORMAT_R64_SFLOAT, { 8, 1, 1 } },
    { VK_FORMAT_R64G64_UINT, { 16, 2, 1 } },
    { VK_FORMAT_R64G64_SINT, { 16, 2, 1 } },
    { VK_FORMAT_R64G64_SFLOAT, { 16, 2, 1 } },
    { VK_FORMAT_R64G64B64_UINT, { 24, 3, 1 } },
    { VK_FORMAT_R64G64B64_SINT, { 24, 3, 1 } },
    { VK_FORMAT_R64G64B64_SFLOAT, { 24, 3, 1 } },
    { VK_FORMAT_R64G64B64A64_UINT, { 32, 4, 1 } },
    { VK_FORMAT_R64G64B64A64_SINT, { 32, 4, 1 } },
    { VK_FORMAT_R64G64B64A64_SFLOAT, { 32, 4, 1 } },
    { VK_FORMAT_B10G11R11_UFLOAT_PACK32, { 4, 3, 1 } },
    { VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, { 4, 3, 1 } },
    { VK_FORMAT_D16_UNORM, { 2, 1, 1 } },
    { VK_FORMAT_X8_D24_UNORM_PACK32, { 4, 1, 1 } },
    { VK_FORMAT_D32_SFLOAT, { 4, 1, 1 } },
    { VK_FORMAT_S8_UINT, { 1, 1, 1 } },
    { VK_FORMAT_D16_UNORM_S8_UINT, { 3, 2, 1 } },
    { VK_FORMAT_D24_UNORM_S8_UINT, { 4, 2, 1 } },
    { VK_FORMAT_D32_SFLOAT_S8_UINT, { 8, 2, 1 } },
    { VK_FORMAT_BC1_RGB_UNORM_BLOCK, { 8, 4, 16 } },
    { VK_FORMAT_BC1_RGB_SRGB_BLOCK, { 8, 4, 16 } },

    { VK_FORMAT_BC1_RGBA_UNORM_BLOCK, { 8, 4, 16 } },
    { VK_FORMAT_BC1_RGBA_SRGB_BLOCK, { 8, 4, 16 } },
    { VK_FORMAT_BC2_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC2_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC3_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC3_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC4_UNORM_BLOCK, { 8, 4, 16 } },
    { VK_FORMAT_BC4_SNORM_BLOCK, { 8, 4, 16 } },
    { VK_FORMAT_BC5_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC5_SNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC6H_UFLOAT_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC6H_SFLOAT_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC7_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_BC7_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, { 8, 3, 16 } },
    { VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, { 8, 3, 16 } },
    { VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, { 8, 4, 16 } },
    { VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, { 8, 4, 16 } },
    { VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_EAC_R11_UNORM_BLOCK, { 8, 1, 16 } },
    { VK_FORMAT_EAC_R11_SNORM_BLOCK, { 8, 1, 16 } },
    { VK_FORMAT_EAC_R11G11_UNORM_BLOCK, { 16, 2, 16 } },
    { VK_FORMAT_EAC_R11G11_SNORM_BLOCK, { 16, 2, 16 } },
    { VK_FORMAT_ASTC_4x4_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_4x4_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_5x4_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_5x4_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_5x5_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_5x5_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_6x5_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_6x5_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_6x6_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_6x6_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_8x5_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_8x5_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_8x6_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_8x6_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_8x8_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_8x8_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x5_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x5_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x6_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x6_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x8_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x8_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x10_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_10x10_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_12x10_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_12x10_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_12x12_UNORM_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_ASTC_12x12_SRGB_BLOCK, { 16, 4, 16 } },
    { VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, { 8, 4, 16 } },
    { VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, { 8, 4, 16 } },
    { VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, { 8, 4, 16 } },
    { VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, { 8, 4, 16 } },
    { VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG, { 8, 4, 16 } },
    { VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG, { 8, 4, 16 } },
    { VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG, { 8, 4, 16 } },
    { VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG, { 8, 4, 16 } },
    // KHRsamplerYCbCrconversion extension - single-plane variants
    // 'Pack' formats are normal, uncompressed
    { VK_FORMAT_R10X6_UNORM_PACK16, { 2, 1, 1 } },
    { VK_FORMAT_R10X6G10X6_UNORM_2PACK16, { 4, 2, 1 } },
    { VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, { 8, 4, 1 } },
    { VK_FORMAT_R12X4_UNORM_PACK16, { 2, 1, 1 } },
    { VK_FORMAT_R12X4G12X4_UNORM_2PACK16, { 4, 2, 1 } },
    { VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16, { 8, 4, 1 } },
    // 422 formats encode 2 texels per entry with B, R components shared - treated as compressed w/ 2x1 _BLOCK size
    { VK_FORMAT_G8B8G8R8_422_UNORM, { 4, 4, 1 } },
    { VK_FORMAT_B8G8R8G8_422_UNORM, { 4, 4, 1 } },
    { VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, { 8, 4, 1 } },
    { VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, { 8, 4, 1 } },
    { VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, { 8, 4, 1 } },
    { VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, { 8, 4, 1 } },
    { VK_FORMAT_G16B16G16R16_422_UNORM, { 8, 4, 1 } },
    { VK_FORMAT_B16G16R16G16_422_UNORM, { 8, 4, 1 } },
    // KHRsamplerYCbCrconversion extension - multi-plane variants
    // Formats that 'share' components among texels (420 and 422}}, size represents total bytes for the smallest possible texel _BLOCK
    // 420 share B, R components within a 2x2 texel _BLOCK
    { VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, { 6, 3, 1 } },
    { VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, { 6, 3, 1 } },
    { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, { 12, 3, 1 } },
    { VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, { 12, 3, 1 } },
    { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, { 12, 3, 1 } },
    { VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, { 12, 3, 1 } },
    { VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, { 12, 3, 1 } },
    { VK_FORMAT_G16_B16R16_2PLANE_420_UNORM, { 12, 3, 1 } },
    // 422 share B, R components within a 2x1 texel _BLOCK
    { VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, { 4, 3, 1 } },
    { VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, { 4, 3, 1 } },
    { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, { 8, 3, 1 } },
    { VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, { 8, 3, 1 } },
    { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, { 8, 3, 1 } },
    { VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, { 8, 3, 1 } },
    { VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, { 8, 3, 1 } },
    { VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, { 8, 3, 1 } },
    // 444 do not share
    { VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, { 3, 3, 1 } },
    { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, { 6, 3, 1 } },
    { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, { 6, 3, 1 } },
    { VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM, { 6, 3, 1 } }
};

uint32_t getBytesPerPixel(VkFormat format)
{
    switch (format) {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
        return 1;
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
        return 2;
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
        return 3;
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:

    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:

    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:

        return 4;

    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
        return 8;
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
        return 12;
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return 16;
    default:
        auto it = vkFormatTable.find(format);
        if (it != vkFormatTable.end()) {
            return it->second.size / it->second.pixelsPerBlock;
        }
    }

    return 0;
}

const VkFormatInfo& GetInfo(VkFormat format)
{
    auto it = vkFormatTable.find(format);
    if (it != vkFormatTable.end()) {
        return it->second;
    }

    return formatUndefined;
}

uint32_t getBytesPerBlock(VkFormat format)
{
    return GetInfo(format).size;
}

uint32_t getPixelsPerBlock(VkFormat format)
{
    return GetInfo(format).pixelsPerBlock;
}

VkFormat convertIntoGammaFormat(VkFormat format)
{
    switch (format) {
    default:
        break;
    case VK_FORMAT_R8_UNORM:
        return VK_FORMAT_R8_SRGB;
    case VK_FORMAT_R8G8_UNORM:
        return VK_FORMAT_R8G8_SRGB;
    case VK_FORMAT_R8G8B8_UNORM:
        return VK_FORMAT_R8G8B8_SRGB;
    case VK_FORMAT_B8G8R8_UNORM:
        return VK_FORMAT_B8G8R8_SRGB;
    case VK_FORMAT_R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case VK_FORMAT_BC2_UNORM_BLOCK:
        return VK_FORMAT_BC2_SRGB_BLOCK;
    case VK_FORMAT_BC3_UNORM_BLOCK:
        return VK_FORMAT_BC3_SRGB_BLOCK;
    case VK_FORMAT_BC7_UNORM_BLOCK:
        return VK_FORMAT_BC7_SRGB_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
    }

    return format;
}

VkFormat convertIntoNonGammaFormat(VkFormat format)
{
    switch (format) {
    default:
        break;
    case VK_FORMAT_R8_SRGB:
        return VK_FORMAT_R8_UNORM;
    case VK_FORMAT_R8G8_SRGB:
        return VK_FORMAT_R8G8_UNORM;
    case VK_FORMAT_R8G8B8_SRGB:
        return VK_FORMAT_R8G8B8_UNORM;
    case VK_FORMAT_B8G8R8_SRGB:
        return VK_FORMAT_B8G8R8_UNORM;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_SRGB:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case VK_FORMAT_BC2_SRGB_BLOCK:
        return VK_FORMAT_BC2_UNORM_BLOCK;
    case VK_FORMAT_BC3_SRGB_BLOCK:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case VK_FORMAT_BC7_SRGB_BLOCK:
        return VK_FORMAT_BC7_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    }

    return format;
}

VkFormat setFormatGamma(VkFormat format, bool addGamma)
{
    if (addGamma) {
        return convertIntoGammaFormat(format);
    } else {
        return convertIntoNonGammaFormat(format);
    }
}

uint32_t bitsPerPixel(VkFormat fmt)
{
    auto it = vkFormatTable.find(fmt);
    if (it != vkFormatTable.end()) {
        return (it->second.size * 8) / it->second.pixelsPerBlock;
    }
    return 0;
    // return (uint32_t)BitsPerPixel(TranslateToDxgiFormat(fmt));
}

bool isSrgb(VkFormat format)
{
    switch (format) {
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        return true;
    default:
        return false;
    }
}

bool IsCompressed_ETC2_EAC(VkFormat format)
{
    bool found = false;
    switch (format) {
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        found = true;
        break;
    default:
        break;
    }
    return found;
}

// Return true if format is an ASTC compressed texture format
bool IsCompressed_ASTC_LDR(VkFormat format)
{
    bool found = false;
    switch (format) {
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        found = true;
        break;
    default:
        break;
    }
    return found;
}

// Return true if format is a BC compressed texture format
bool IsCompressed_BC(VkFormat format)
{
    bool found = false;
    switch (format) {
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
        found = true;
        break;
    default:
        break;
    }
    return found;
}

// Return true if format is a PVRTC compressed texture format
bool IsCompressed_PVRTC(VkFormat format)
{
    bool found = false;
    switch (format) {
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        found = true;
        break;
    default:
        break;
    }
    return found;
}

// Single-plane "422" formats are treated as 2x1 compressed (for copies)
bool IsSinglePLANE422(VkFormat format)
{
    bool found = false;
    switch (format) {
    case VK_FORMAT_G8B8G8R8_422_UNORM_KHR:
    case VK_FORMAT_B8G8R8G8_422_UNORM_KHR:
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR:
    case VK_FORMAT_G16B16G16R16_422_UNORM_KHR:
    case VK_FORMAT_B16G16R16G16_422_UNORM_KHR:
        found = true;
        break;
    default:
        break;
    }
    return found;
}

// Return true if format is compressed
bool IsCompressed(VkFormat format)
{
    return (IsCompressed_ASTC_LDR(format) || IsCompressed_BC(format) || IsCompressed_ETC2_EAC(format) || IsCompressed_PVRTC(format));
}


}