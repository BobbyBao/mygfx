#pragma once
#include <stdint.h>
#include "GraphicsDefs.h"
#include "utils/RefCounted.h"
#include <span>
#include <vector>

namespace mygfx {
	
    using MemoryBlock = std::span<uint8_t>;
	
    template<typename T>
    struct ImageFace
    {
        uint8_t* pixelData;
        int width, height, depth;

        ImageFace(uint8_t* data, int w, int h, int d)
        {
            pixelData = data;
            width = (int)w;
            height = (int)h;
            depth = (int)d;
        }
          
        T& sample(float x, float y)
        {
            return this->at((int)(x * width), (int)(y * height));
        }

        T& at(int x, int y)
        {
            x = std::max(0, std::min(x, (int)width - 1));
            y = std::max(0, std::min(y, (int)height - 1));
            return ((T*)pixelData)[(x + y * width)];
        }

        T& at(int x, int y, int z)
        {
            x = std::max(0, std::min(x, (int)width - 1));
            y = std::max(0, std::min(y, (int)height - 1));
            z = std::max(0, std::min(z, (int)depth - 1));
            return ((T*)pixelData)[(x + (y + z * depth) * width)];
        }

    };

    struct TextureData
    {
        static TextureData Texture2D(uint32_t w, uint32_t h, Format fmt, const MemoryBlock& data = {});
        static TextureData Texture2D(uint32_t w, uint32_t h, int channels, const MemoryBlock& data = {});
		static TextureData Texture2DArray(uint32_t w, uint32_t h, uint32_t layers, Format fmt);
		static TextureData Texture3D(uint32_t w, uint32_t h, uint32_t depth, Format fmt);
		static TextureData TextureCube(uint32_t w, uint32_t h, uint32_t layers, Format fmt);

        inline uint8_t* data() { return (uint8_t*)dataBlock.data(); }
        inline const uint8_t* data() const { return (uint8_t*)dataBlock.data(); }
        inline bool isCubemap() const { return faceCount > 1; }
        inline bool isArray() const { return layerCount > 1; }

	    uint32_t bitsPerPixel() const;        
        uint32_t bitsPerBlock() const;
        uint32_t pixelsPerBlock() const;

        uint32_t getImageSize(uint16_t level);
        size_t getLayerSize(uint16_t level);
        size_t getLevelSize(uint16_t level);
        size_t getDataSize(uint16_t levels);
        bool getImageOffset(uint16_t level, uint16_t layer, uint16_t faceSlice, size_t* pOffset, size_t* pImageSize = nullptr);
        size_t getTotalSize();

        std::span<uint8_t> getSpan(uint16_t level = 0, uint16_t layer = 0, uint16_t face = 0);

        template<typename T>
        ImageFace<T> getFace(uint16_t level = 0, uint16_t layer = 0, uint16_t face = 0)
        {
            size_t offset;
            if (!getImageOffset(level, layer, face, &offset)) {
                return {};
            }

            uint32_t mipWidth = std::max(1, width >> level);
            uint32_t mipHeight = std::max(1, height >> level);
            uint32_t mipDepth = std::max(1, depth >> level);
            return ImageFace<T>((dataBlock.data() + offset), mipWidth, mipHeight, mipDepth);
        }

        template<typename T>
        ImageFace<T> asFace()
        {
            assert(mipMapCount == 1 && layerCount == 1 && faceCount == 1);
            return ImageFace<T>(dataBlock.data(), width, height, depth);
        }
        
		auto operator<=>(TextureData const&) const = default;

        String name;
        uint16_t width = 0;
        uint16_t height = 0;
        uint16_t depth = 1;
        uint16_t layerCount = 1;
        uint16_t faceCount = 1;
        uint16_t mipMapCount = 1;
		Format format = Format::UNDEFINED;
		TextureUsage usage = TextureUsage::NONE;
		SamplerType samplerType = SamplerType::COUNT;
        SampleCount sampleCount = SampleCount::SAMPLE_1;
        MemoryBlock dataBlock;
    private:
    };
    
    class TextureDataProvider {
    public:
        virtual void copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) = 0;
    };

}