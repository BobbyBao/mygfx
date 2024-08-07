#pragma once
#include "TextureData.h"

namespace mygfx {
	
	class Texture;

	class TextureLoader : public TextureDataProvider {
	public:
		TextureLoader() = default;
		virtual ~TextureLoader() {}

		Ref<Texture> load(const String& fileName, SamplerInfo samplerInfo);
		virtual Ref<Texture> onLoad(const Span<uint8_t>& content, SamplerInfo samplerInfo) = 0;

	protected:
	};

	class StbTextureLoader : public TextureLoader {
	public:
		StbTextureLoader() = default;
		~StbTextureLoader();

		Ref<Texture> onLoad(const Span<uint8_t>& content, SamplerInfo samplerInfo) override;
	protected:
		void copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) override;
		void mipImage(uint32_t width, uint32_t height);
		
		union
		{
			float* mHdrData = nullptr;
			char* mData;
		};

		bool mIsHdr = false;
	};

	class KtxTextureLoader : public TextureLoader {
	public:
		KtxTextureLoader() = default;
		~KtxTextureLoader();

		Ref<Texture> onLoad(const Span<uint8_t>& content, SamplerInfo samplerInfo) override;
	protected:
		void copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) override;

		void* mKtxTexture = nullptr;
		uint8_t* ktxTextureData = nullptr;
		size_t ktxTextureSize;
	};

	class DDSTextureLoader : public TextureLoader {
	public:
		DDSTextureLoader() = default;
		~DDSTextureLoader();

		Ref<Texture> onLoad(const Span<uint8_t>& content, SamplerInfo samplerInfo) override;
	protected:
		void copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) override;
		uint8_t* mData = nullptr;
	};

}

