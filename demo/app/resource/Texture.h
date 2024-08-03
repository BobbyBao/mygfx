#pragma once
#include "GraphicsHandles.h"
#include "TextureData.h"
#include "MathTypes.h"

namespace mygfx {
	
	class Texture : public RefCounted {
	public:
		Texture();
		
		size_t getLevelCount() const noexcept { return mTextureData.mipMapCount; }
		SamplerInfo getSampler() const noexcept { return mSamplerInfo; }
		Format getFormat() const noexcept { return mTextureData.format; }
		TextureUsage getUsage() const noexcept { return mTextureData.usage; }

		int index() const;
		auto getSRV() const { return mHwTexture->getSRV(); }
		auto getRTV() const { return mHwTexture->getRTV(); }
		auto getDSV() const { return mHwTexture->getDSV(); }

		TextureData& textureData() { return mTextureData; }
		const Ref<HwTexture>& getHwTexture() const { return mHwTexture; }
		
		bool create(const TextureData& textureData, SamplerInfo samplerInfo = {});

		void copyData(TextureDataProvider* dataProvider);

		void setData(uint32_t level,
			uint32_t xoffset,
			uint32_t yoffset,
			uint32_t zoffset,
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			const void* data,
			size_t size);

        static void staticInit();
        static void staticDeinit();
		
		// Returns the with or height for a given mipmap level from the base value.
		static inline size_t valueForLevel(uint8_t level, size_t baseLevelValue) {
			return std::max(size_t(1), baseLevelValue >> level);
		}

		// Returns the max number of levels for a texture of given max dimensions
		static inline uint8_t maxLevelCount(uint32_t maxDimension) noexcept {
			return std::max(1, std::ilogbf(float(maxDimension)) + 1);
		}

		// Returns the max number of levels for a texture of given dimensions
		static inline uint8_t maxLevelCount(uint32_t width, uint32_t height) noexcept {
			uint32_t const maxDimension = std::max(width, height);
			return maxLevelCount(maxDimension);
		}

		static Ref<Texture> create2D(uint16_t width, uint16_t height, Format format, const MemoryBlock& memoryBlock, SamplerInfo samplerInfo = {});
		static Ref<Texture> createFromData(const TextureData& imageInfo, SamplerInfo samplerInfo = {}); 
		static Ref<Texture> createFromFile(const String& fileName, SamplerInfo samplerInfo = {});
		static Ref<Texture> createFromData(const Span<uint8_t>& content, const String& type, SamplerInfo samplerInfo = {});
		
		static Ref<Texture> createRenderTarget(uint16_t width, uint16_t height, Format format, TextureUsage usage = TextureUsage::NONE, SampleCount msaa = SampleCount::SAMPLE_1);
		static Ref<Texture> createDepthStencil(uint16_t width, uint16_t height, Format format, TextureUsage usage = TextureUsage::NONE, bool isShadowMap = false, SampleCount msaa = SampleCount::SAMPLE_1);
	
		static Ref<Texture> createByColor(const char* name, const float4& color);
		static Vector<Ref<Texture>> createRandomColorTextures(int count);

		static Ref<Texture> White;
		static Ref<Texture> Black;
		static Ref<Texture> Gray;
        static Ref<Texture> Red;
        static Ref<Texture> Green;
        static Ref<Texture> Blue;
        static Ref<Texture> Yellow;
		static Ref<Texture> Cyan;
		static Ref<Texture> Magenta;
	private:
		bool onCreate();

		TextureData mTextureData;
		SamplerInfo mSamplerInfo;
		Ref<HwTexture> mHwTexture;
	};
}
