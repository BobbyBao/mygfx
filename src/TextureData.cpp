#include "TextureData.h"
#include "Format.h"
#include "GraphicsDevice.h"

namespace mygfx {

#define _KTX_PADN(n, nbytes) (nbytes + (n - 1) & ~(uint32_t)(n - 1))
#define _KTX_PADN_LEN(n, nbytes) ((n - 1) - (nbytes + (n - 1) & (n - 1)))

static uint32_t padRow(uint32_t* rowBytes)
{
    uint32_t rowPadding;

    assert(rowBytes != NULL);

    rowPadding = _KTX_PADN_LEN(4, *rowBytes);
    *rowBytes += rowPadding;
    return rowPadding;
}

TextureData TextureData::texture2D(uint32_t w, uint32_t h, Format fmt, const MemoryBlock& data)
{
    return TextureData {
        .width = (uint16_t)w,
        .height = (uint16_t)h,
        .format = fmt,
        .samplerType = SamplerType::SAMPLER_2D,
        .dataBlock = data,
    };
}

TextureData TextureData::texture2DArray(uint32_t w, uint32_t h, uint32_t layers, Format fmt)
{
    return TextureData {
        .width = (uint16_t)w,
        .height = (uint16_t)h,
        .layerCount = (uint16_t)layers,
        .format = fmt,
        .samplerType = SamplerType::SAMPLER_2D_ARRAY,
    };
}

TextureData TextureData::texture3D(uint32_t w, uint32_t h, uint32_t depth, Format fmt)
{
    return TextureData {
        .width = (uint16_t)w,
        .height = (uint16_t)h,
        .depth = (uint16_t)depth,
        .format = fmt,
        .samplerType = SamplerType::SAMPLER_3D,
    };
}

TextureData TextureData::textureCube(uint32_t w, uint32_t h, uint32_t layers, Format fmt)
{
    return TextureData {
        .width = (uint16_t)w,
        .height = (uint16_t)h,
        .layerCount = (uint16_t)layers,
        .faceCount = 6,
        .format = fmt,
        .samplerType = layers == 1 ? SamplerType::SAMPLER_CUBE : SamplerType::SAMPLER_CUBE_ARRAY,
    };
}

TextureData TextureData::texture2D(uint32_t w, uint32_t h, int channels, const MemoryBlock& data)
{
    Format fmt = Format::UNDEFINED;
    switch (channels) {
    case 1:
        fmt = Format::R8_UNORM;
        break;
    case 2:
        fmt = Format::R8G8_UNORM;
        break;
    case 3:
        fmt = Format::R8G8B8_UNORM;
        break;
    case 4:
        fmt = Format::R8G8B8A8_UNORM;
        break;
    default:
        assert(false);
        break;
    };

    return TextureData {
        .width = (uint16_t)w,
        .height = (uint16_t)h,
        .format = fmt,
        .samplerType = SamplerType::SAMPLER_2D,
        .dataBlock = data
    };
}

uint32_t TextureData::bitsPerPixel() const
{
    return getBitsPerPixel(format);
}

uint32_t TextureData::bitsPerBlock() const
{
    return getPixelsPerBlock(format);
}

uint32_t TextureData::pixelsPerBlock() const
{
    return getPixelsPerBlock(format);
}

std::span<uint8_t> TextureData::getSpan(uint16_t level, uint16_t layer, uint16_t face)
{
    size_t offset;
    size_t imageSize;
    if (!getImageOffset(level, layer, face, &offset, &imageSize)) {
        return {};
    }

    return std::span<uint8_t>((uint8_t*)dataBlock.data() + offset, imageSize);
}

uint32_t TextureData::getImageSize(uint16_t level)
{
    const FormatInfo& formatInfo = getFormatInfo(format);

    struct blockCount {
        uint32_t x, y, z;
    } blockCount;

    uint32_t blockSizeInBytes;
    uint32_t rowBytes;

    float levelWidth = (float)(width >> level);
    float levelHeight = (float)(height >> level);
    blockCount.x = (uint32_t)ceilf(levelWidth / formatInfo.blockWidth);
    blockCount.y = (uint32_t)ceilf(levelHeight / formatInfo.blockHeight);
    blockCount.x = std::max(1u, blockCount.x);
    blockCount.y = std::max(1u, blockCount.y);
    blockSizeInBytes = formatInfo.blockSizeInBits / 8;

    if (formatInfo.compressed) {
        return blockCount.x * blockCount.y * blockSizeInBytes;
    } else {
        assert(formatInfo.blockWidth == formatInfo.blockHeight == formatInfo.blockDepth == 1);
        rowBytes = blockCount.x * blockSizeInBytes;
        (void)padRow(&rowBytes);
        return rowBytes * blockCount.y;
    }

    uint16_t mipWidth = std::max(1, width >> level);
    uint16_t mipHeight = std::max(1, height >> level);
    uint16_t mipDepth = std::max(1, depth >> level);
    return (mipWidth * mipHeight * mipDepth * bitsPerPixel()) / (pixelsPerBlock() * 8);
}

size_t TextureData::getLayerSize(uint16_t level)
{
    /*
     * As there are no 3D cubemaps, the image's z block count will always be
     * 1 for cubemaps and numFaces will always be 1 for 3D textures so the
     * multiply is safe. 3D cubemaps, if they existed, would require
     * imageSize * (blockCount.z + This->numFaces);
     */
    uint32_t blockCountZ;
    size_t imageSize, layerSize;

    const FormatInfo& formatInfo = getFormatInfo(format);
    blockCountZ = std::max(1, (depth / formatInfo.blockDepth) >> level);
    imageSize = getImageSize(level);
    layerSize = imageSize * blockCountZ;
    return layerSize * faceCount;
}

size_t TextureData::getLevelSize(uint16_t level)
{
    return getLayerSize(level) * layerCount;
}

size_t TextureData::getDataSize(uint16_t levels)
{
    size_t dataSize = 0;
    for (uint32_t i = 0; i < levels; i++) {
        size_t levelSize = getLevelSize(i);
        dataSize += levelSize;
    }

    return dataSize;
}

bool TextureData::getImageOffset(uint16_t level, uint16_t layer, uint16_t faceSlice, size_t* pOffset, size_t* pImageSize)
{
    if (level >= mipMapCount || layer >= layerCount)
        return false;

    if (isCubemap()) {
        if (faceSlice >= faceCount)
            return false;
    } else {
        uint32_t maxSlice = std::max(1, depth >> level);
        if (faceSlice >= maxSlice)
            return false;
    }

    // Get the size of the data up to the start of the indexed level.
    *pOffset = getDataSize(level);

    // All layers, faces & slices within a level are the same size.
    if (layer != 0) {
        size_t layerSize;
        layerSize = getLayerSize(level);
        *pOffset += layer * layerSize;
    }

    if (faceSlice != 0) {
        size_t imageSize = getImageSize(level);
        *pOffset += faceSlice * imageSize;

        if (pImageSize) {
            *pImageSize = imageSize;
        }
    } else {

        if (pImageSize) {
            *pImageSize = getImageSize(level);
        }
    }

    return true;
}

size_t TextureData::getTotalSize()
{
    size_t size = 0;
    for (uint32_t i = 0; i < mipMapCount; i++) {
        size += getLevelSize(i);
    }
    return size;
}
}