#include "Texture.h"
#include "GraphicsDevice.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "utils/FileUtils.h"
#include <ktx.h>

namespace mygfx {

	struct KtxTextureLoader : public TextureDataProvider {

		char* mData = nullptr;

		KtxTextureLoader() = default;
		~KtxTextureLoader();
		Ref<Texture> load(const String& fileName, SamplerInfo samplerInfo);
		void copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) override;

	};

	struct StbTextureLoader : public TextureDataProvider {

		char* mData = nullptr;
		StbTextureLoader() = default;
		~StbTextureLoader();
		Ref<Texture> load(const String& fileName, SamplerInfo samplerInfo);
		void copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) override;
		void mipImage(uint32_t width, uint32_t height);
		
	};

	Ref<Texture> Texture::White;
	Ref<Texture> Texture::Black;
	Ref<Texture> Texture::Gray;
	Ref<Texture> Texture::Red;
    Ref<Texture> Texture::Green;
    Ref<Texture> Texture::Blue;
    Ref<Texture> Texture::Yellow;
	Ref<Texture> Texture::Cyan;
	Ref<Texture> Texture::Magenta;

	void Texture::staticInit()
	{
		White = Texture::createByColor("White", float4{ 1.0f, 1.0f, 1.0f, 1.0f });
		Black = Texture::createByColor("Black", float4{ 0.0f, 0.0f, 0.0f, 1.0f });
		Gray = Texture::createByColor("Gray", float4{ 0.5f, 0.5f, 0.5f, 1.0f });
		Red = Texture::createByColor("Red", float4{ 1.0f, 0.0f, 0.0f, 1.0f });
		Green = Texture::createByColor("Green", float4{ 0.0f, 1.0f, 0.0f, 1.0f });
		Blue = Texture::createByColor("Blue", float4{ 0.0f, 0.0f, 1.0f, 1.0f });
		Yellow = Texture::createByColor("Yellow", float4{ 1.0f, 1.0f, 0.0f, 1.0f });
		Cyan = Texture::createByColor("Cyan", float4{0.0f,  1.0f, 1.0f, 1.0f });
		Magenta = Texture::createByColor("Magenta", float4{ 1.0f, 0.0f, 1.0f, 1.0f });

	}

	void Texture::staticDeinit()
	{
		White = nullptr;
		Gray = nullptr;
		Black = nullptr;
		Red = nullptr;
		Green = nullptr;
		Blue = nullptr;
		Yellow = nullptr;
		Cyan = nullptr;
		Magenta = nullptr;
	}
	
	Texture::Texture() = default;

	int Texture::index() const { return mHwTexture ? mHwTexture->getSRV()->index() : 0; }
	
	bool Texture::create(const TextureData& textureData, SamplerInfo samplerInfo) {
		mSamplerInfo = samplerInfo;
		mTextureData = textureData;
		return onCreate();
	}

	bool Texture::onCreate() {
		mHwTexture = device().createTexture(mTextureData, mSamplerInfo);
		return mHwTexture != nullptr;
	}
		
	void Texture::copyData(TextureDataProvider* dataProvider) {
		device().copyData(mHwTexture, dataProvider);
	}

	void Texture::setData(uint32_t level,
		uint32_t xoffset,
		uint32_t yoffset,
		uint32_t zoffset,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		const void* data,
		size_t size) {

		device().updateTexture(mHwTexture, level, xoffset,
			yoffset,
			zoffset,
			width,
			height,
			depth,
			data,
			size);
	}

	unsigned toUInt(const float4& c)
	{
		auto r_ = (unsigned)math::clamp(((int)(c.x * 255.0f)), 0, 255);
		auto g_ = (unsigned)math::clamp(((int)(c.y * 255.0f)), 0, 255);
		auto b_ = (unsigned)math::clamp(((int)(c.z * 255.0f)), 0, 255);
		auto a_ = (unsigned)math::clamp(((int)(c.w * 255.0f)), 0, 255);
		return (a_ << 24u) | (b_ << 16u) | (g_ << 8u) | r_;
	}

	Ref<Texture> Texture::createByColor(const char* name, const float4& color)
	{
		auto c = toUInt(color);
		MemoryBlock memoryBlock((uint8_t*) & c, 4);;
		auto textureData = TextureData::Texture2D(1, 1, Format::R8G8B8A8_UNORM, memoryBlock);
		textureData.usage = TextureUsage::Sampled | TextureUsage::TransferDst;
		textureData.name = name;
		auto tex = createFromData(textureData);
		return tex;
	}

	Ref<Texture> Texture::create2D(uint16_t width, uint16_t height, Format format, const MemoryBlock& memoryBlock, SamplerInfo samplerInfo)
	{
		auto textureData = TextureData::Texture2D(width, height, format, memoryBlock);
		textureData.usage = TextureUsage::Sampled | TextureUsage::TransferDst;
		return createFromData(textureData, samplerInfo);
	}

	Ref<Texture> Texture::createFromData(const TextureData& textureData, SamplerInfo samplerInfo)
	{
		Ref<Texture> tex(new Texture());
		if(!tex->create(textureData, samplerInfo)) {
			return nullptr;
		}
		return tex;
	}

	Ref<Texture> Texture::createFromFile(const String& fileName, SamplerInfo samplerInfo)
	{
		if (fileName.ends_with(".ktx")) {
			KtxTextureLoader dataProvider;
			return dataProvider.load(fileName, samplerInfo);
		}

		StbTextureLoader dataProvider;
		return dataProvider.load(fileName, samplerInfo);
	}

	Ref<Texture> Texture::createRenderTarget(uint16_t width, uint16_t height, Format format, TextureUsage usage, SampleCount msaa) {
		auto textureData = TextureData::Texture2D(width, height, format);
		textureData.sampleCount = msaa;
		textureData.usage = TextureUsage::ColorAttachment | usage;
		return createFromData(textureData, SamplerInfo{Filter::Nearest, SamplerAddressMode::ClampToEdge});
	}

	Ref<Texture> Texture::createDepthStencil(uint16_t width, uint16_t height, Format format, TextureUsage usage, bool isShadowMap, SampleCount msaa) {
		auto textureData = TextureData::Texture2D(width, height, format);
		textureData.sampleCount = msaa;
		textureData.usage = TextureUsage::DepthStencilAttachment | usage;

		SamplerInfo samplerInfo{Filter::Nearest, SamplerAddressMode::ClampToEdge};
		
		if (isShadowMap) {
			textureData.usage |= TextureUsage::Sampled;
			samplerInfo.compareEnable = true;
			samplerInfo.compareOp = CompareOp::LessOrEqual;
		}

		return createFromData(textureData, samplerInfo);
	}


	StbTextureLoader::~StbTextureLoader() {
		if (mData)
			stbi_image_free(mData);
	}

	Ref<Texture> StbTextureLoader::load(const String& fileName, SamplerInfo samplerInfo) {
		TextureData textureData;
		auto content = FileUtils::readAll(fileName);

		int32_t width, height, channels;
		mData = (char*)stbi_load_from_memory(content.data(), (int)content.size(), &width, &height, &channels, STBI_rgb_alpha);
		if (!mData) {
			return nullptr;
		}

		// compute number of mips
		//
		uint32_t mipWidth = width;
		uint32_t mipHeight = height;
		uint32_t mipCount = 0;
		for (;;) {
			mipCount++;
			if (mipWidth > 1) mipWidth >>= 1;
			if (mipHeight > 1) mipHeight >>= 1;
			if (mipWidth == 1 && mipHeight == 1)
				break;
		}

		// fill img struct
		//
		textureData.layerCount = 1;
		textureData.width = width;
		textureData.height = height;
		textureData.depth = 1;
		textureData.mipMapCount = mipCount;
		textureData.format = Format::R8G8B8A8_UNORM;

		Ref<Texture> tex(new Texture());
		if (!tex->create(textureData, samplerInfo)) {
			return nullptr;
		}

		tex->copyData(this);
		return tex;
	}

	void StbTextureLoader::copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) {

		memcpy((char*)pDest, mData, imageSize);
		mipImage(width, height);
	}


	void StbTextureLoader::mipImage(uint32_t width, uint32_t height)
	{
		//compute mip so next call gets the lower mip
		int offsetsX[] = { 0,1,0,1 };
		int offsetsY[] = { 0,0,1,1 };

		uint32_t* pImg = (uint32_t*)mData;

#define GetByte(color, component) (((color) >> (8 * (component))) & 0xff)
#define GetColor(ptr, x,y) (ptr[(x)+(y)*width])
#define SetColor(ptr, x,y, col) ptr[(x)+(y)*width/2]=col;

		for (uint32_t y = 0; y < height; y += 2) {
			for (uint32_t x = 0; x < width; x += 2) {
				uint32_t ccc = 0;
				for (uint32_t c = 0; c < 4; c++) {
					uint32_t cc = 0;
					for (uint32_t i = 0; i < 4; i++)
						cc += GetByte(GetColor(pImg, x + offsetsX[i], y + offsetsY[i]), 3 - c);

					ccc = (ccc << 8) | (cc / 4);
				}
				SetColor(pImg, x / 2, y / 2, ccc);
			}
		}
	}

	KtxTextureLoader::~KtxTextureLoader() {

	}

	Ref<Texture> KtxTextureLoader::load(const String& fileName, SamplerInfo samplerInfo) {
		return nullptr;
	}

	void KtxTextureLoader::copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) {
	}


}