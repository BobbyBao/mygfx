#include "Texture.h"
#include "GraphicsDevice.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "utils/FileUtils.h"
#include <ktx.h>
#include <gl_format.h>

namespace mygfx {

	struct KtxTextureLoader : public TextureDataProvider {
		ktxTexture* ktxTexture = nullptr;
		ktx_uint8_t* ktxTextureData = nullptr;
		ktx_size_t ktxTextureSize;

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
		Cyan = Texture::createByColor("Cyan", float4{ 0.0f,  1.0f, 1.0f, 1.0f });
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
		MemoryBlock memoryBlock((uint8_t*)&c, 4);;
		auto textureData = TextureData::Texture2D(1, 1, Format::R8G8B8A8_UNORM, memoryBlock);
		textureData.usage = TextureUsage::SAMPLED | TextureUsage::TRANSFER_DST;
		textureData.name = name;
		auto tex = createFromData(textureData);
		return tex;
	}

	Ref<Texture> Texture::create2D(uint16_t width, uint16_t height, Format format, const MemoryBlock& memoryBlock, SamplerInfo samplerInfo)
	{
		auto textureData = TextureData::Texture2D(width, height, format, memoryBlock);
		textureData.usage = TextureUsage::SAMPLED | TextureUsage::TRANSFER_DST;
		return createFromData(textureData, samplerInfo);
	}

	Ref<Texture> Texture::createFromData(const TextureData& textureData, SamplerInfo samplerInfo)
	{
		Ref<Texture> tex(new Texture());
		if (!tex->create(textureData, samplerInfo)) {
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
		textureData.usage = TextureUsage::COLOR_ATTACHMENT | usage;
		return createFromData(textureData, SamplerInfo{ Filter::NEAREST, SamplerAddressMode::CLAMP_TO_EDGE });
	}

	Ref<Texture> Texture::createDepthStencil(uint16_t width, uint16_t height, Format format, TextureUsage usage, bool isShadowMap, SampleCount msaa) {
		auto textureData = TextureData::Texture2D(width, height, format);
		textureData.sampleCount = msaa;
		textureData.usage = TextureUsage::DEPTH_STENCIL_ATTACHMENT | usage;

		SamplerInfo samplerInfo{ Filter::NEAREST, SamplerAddressMode::CLAMP_TO_EDGE };

		if (isShadowMap) {
			textureData.usage |= TextureUsage::SAMPLED;
			samplerInfo.compareEnable = true;
			samplerInfo.compareOp = CompareOp::LESS_OR_EQUAL;
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

		uint32_t mipCount = Texture::maxLevelCount(width, height);

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

	static inline Format GetFormatFromOpenGLInternalFormat(const GLenum internalFormat)
	{
		switch (internalFormat)
		{
			//
			// 8 bits per component
			//
		case GL_R8:												return Format::R8_UNORM;					// 1-component, 8-bit unsigned normalized
		case GL_RG8:											return Format::R8G8_UNORM;				// 2-component, 8-bit unsigned normalized
		case GL_RGB8:											return Format::R8G8B8_UNORM;				// 3-component, 8-bit unsigned normalized
		case GL_RGBA8:											return Format::R8G8B8A8_UNORM;			// 4-component, 8-bit unsigned normalized

		case GL_R8_SNORM:										return Format::R8_SNORM;					// 1-component, 8-bit signed normalized
		case GL_RG8_SNORM:										return Format::R8G8_SNORM;				// 2-component, 8-bit signed normalized
		case GL_RGB8_SNORM:										return Format::R8G8B8_SNORM;				// 3-component, 8-bit signed normalized
		case GL_RGBA8_SNORM:									return Format::R8G8B8A8_SNORM;			// 4-component, 8-bit signed normalized

		case GL_R8UI:											return Format::R8_UINT;					// 1-component, 8-bit unsigned integer
		case GL_RG8UI:											return Format::R8G8_UINT;					// 2-component, 8-bit unsigned integer
		case GL_RGB8UI:											return Format::R8G8B8_UINT;				// 3-component, 8-bit unsigned integer
		case GL_RGBA8UI:										return Format::R8G8B8A8_UINT;				// 4-component, 8-bit unsigned integer

		case GL_R8I:											return Format::R8_SINT;					// 1-component, 8-bit signed integer
		case GL_RG8I:											return Format::R8G8_SINT;					// 2-component, 8-bit signed integer
		case GL_RGB8I:											return Format::R8G8B8_SINT;				// 3-component, 8-bit signed integer
		case GL_RGBA8I:											return Format::R8G8B8A8_SINT;				// 4-component, 8-bit signed integer

		case GL_SR8:											return Format::R8_SRGB;					// 1-component, 8-bit sRGB
		case GL_SRG8:											return Format::R8G8_SRGB;					// 2-component, 8-bit sRGB
		case GL_SRGB8:											return Format::R8G8B8_SRGB;				// 3-component, 8-bit sRGB
		case GL_SRGB8_ALPHA8:									return Format::R8G8B8A8_SRGB;				// 4-component, 8-bit sRGB

			//
			// 16 bits per component
			//
		case GL_R16:											return Format::R16_UNORM;					// 1-component, 16-bit unsigned normalized
		case GL_RG16:											return Format::R16G16_UNORM;				// 2-component, 16-bit unsigned normalized
		case GL_RGB16:											return Format::R16G16B16_UNORM;			// 3-component, 16-bit unsigned normalized
		case GL_RGBA16:											return Format::R16G16B16A16_UNORM;		// 4-component, 16-bit unsigned normalized

		case GL_R16_SNORM:										return Format::R16_SNORM;					// 1-component, 16-bit signed normalized
		case GL_RG16_SNORM:										return Format::R16G16_SNORM;				// 2-component, 16-bit signed normalized
		case GL_RGB16_SNORM:									return Format::R16G16B16_SNORM;			// 3-component, 16-bit signed normalized
		case GL_RGBA16_SNORM:									return Format::R16G16B16A16_SNORM;		// 4-component, 16-bit signed normalized

		case GL_R16UI:											return Format::R16_UINT;					// 1-component, 16-bit unsigned integer
		case GL_RG16UI:											return Format::R16G16_UINT;				// 2-component, 16-bit unsigned integer
		case GL_RGB16UI:										return Format::R16G16B16_UINT;			// 3-component, 16-bit unsigned integer
		case GL_RGBA16UI:										return Format::R16G16B16A16_UINT;			// 4-component, 16-bit unsigned integer

		case GL_R16I:											return Format::R16_SINT;					// 1-component, 16-bit signed integer
		case GL_RG16I:											return Format::R16G16_SINT;				// 2-component, 16-bit signed integer
		case GL_RGB16I:											return Format::R16G16B16_SINT;			// 3-component, 16-bit signed integer
		case GL_RGBA16I:										return Format::R16G16B16A16_SINT;			// 4-component, 16-bit signed integer

		case GL_R16F:											return Format::R16_SFLOAT;				// 1-component, 16-bit floating-point
		case GL_RG16F:											return Format::R16G16_SFLOAT;				// 2-component, 16-bit floating-point
		case GL_RGB16F:											return Format::R16G16B16_SFLOAT;			// 3-component, 16-bit floating-point
		case GL_RGBA16F:										return Format::R16G16B16A16_SFLOAT;		// 4-component, 16-bit floating-point

			//
			// 32 bits per component
			//
		case GL_R32UI:											return Format::R32_UINT;					// 1-component, 32-bit unsigned integer
		case GL_RG32UI:											return Format::R32G32_UINT;				// 2-component, 32-bit unsigned integer
		case GL_RGB32UI:										return Format::R32G32B32_UINT;			// 3-component, 32-bit unsigned integer
		case GL_RGBA32UI:										return Format::R32G32B32A32_UINT;			// 4-component, 32-bit unsigned integer

		case GL_R32I:											return Format::R32_SINT;					// 1-component, 32-bit signed integer
		case GL_RG32I:											return Format::R32G32_SINT;				// 2-component, 32-bit signed integer
		case GL_RGB32I:											return Format::R32G32B32_SINT;			// 3-component, 32-bit signed integer
		case GL_RGBA32I:										return Format::R32G32B32A32_SINT;			// 4-component, 32-bit signed integer

		case GL_R32F:											return Format::R32_SFLOAT;				// 1-component, 32-bit floating-point
		case GL_RG32F:											return Format::R32G32_SFLOAT;				// 2-component, 32-bit floating-point
		case GL_RGB32F:											return Format::R32G32B32_SFLOAT;			// 3-component, 32-bit floating-point
		case GL_RGBA32F:										return Format::R32G32B32A32_SFLOAT;		// 4-component, 32-bit floating-point

			//
			// Packed
			//
		case GL_R3_G3_B2:										return Format::UNDEFINED;					// 3-component 3:3:2,       unsigned normalized
		case GL_RGB4:											return Format::UNDEFINED;					// 3-component 4:4:4,       unsigned normalized
		case GL_RGB5:											return Format::R5G5B5A1_UNORM;		// 3-component 5:5:5,       unsigned normalized
		case GL_RGB565:											return Format::R5G6B5_UNORM;		// 3-component 5:6:5,       unsigned normalized
		case GL_RGB10:											return Format::A2R10G10B10_UNORM;	// 3-component 10:10:10,    unsigned normalized
		case GL_RGB12:											return Format::UNDEFINED;					// 3-component 12:12:12,    unsigned normalized
		case GL_RGBA2:											return Format::UNDEFINED;					// 4-component 2:2:2:2,     unsigned normalized
		case GL_RGBA4:											return Format::R4G4B4A4_UNORM;		// 4-component 4:4:4:4,     unsigned normalized
		case GL_RGBA12:											return Format::UNDEFINED;					// 4-component 12:12:12:12, unsigned normalized
		case GL_RGB5_A1:										return Format::A1R5G5B5_UNORM;		// 4-component 5:5:5:1,     unsigned normalized
		case GL_RGB10_A2:										return Format::A2R10G10B10_UNORM;	// 4-component 10:10:10:2,  unsigned normalized
		case GL_RGB10_A2UI:										return Format::A2R10G10B10_UINT;	// 4-component 10:10:10:2,  unsigned integer
		case GL_R11F_G11F_B10F:									return Format::B10G11R11_UFLOAT;	// 3-component 11:11:10,    floating-point
		case GL_RGB9_E5:										return Format::E5B9G9R9_UFLOAT;	// 3-component/exp 9:9:9/5, floating-point

			//
			// S3TC/DXT/BC
			//

		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:					return Format::DXBC1_RGB_UNORM;		// line through 3D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:					return Format::DXBC1_RGBA_UNORM;		// line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:					return Format::DXBC2_UNORM;			// line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:					return Format::DXBC3_UNORM;			// line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:					return Format::DXBC1_RGB_SRGB;		// line through 3D space, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:			return Format::DXBC1_RGBA_SRGB;		// line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:			return Format::DXBC2_SRGB;			// line through 3D space plus line through 1D space, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:			return Format::DXBC3_SRGB;			// line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

		case GL_COMPRESSED_LUMINANCE_LATC1_EXT:					return Format::DXBC4_UNORM;			// line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:			return Format::DXBC5_UNORM;			// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:			return Format::DXBC4_SNORM;			// line through 1D space, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:	return Format::DXBC5_SNORM;			// two lines through 1D space, 4x4 blocks, signed normalized

		case GL_COMPRESSED_RED_RGTC1:							return Format::DXBC4_UNORM;			// line through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RG_RGTC2:							return Format::DXBC5_UNORM;			// two lines through 1D space, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_RED_RGTC1:					return Format::DXBC4_SNORM;			// line through 1D space, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_RG_RGTC2:						return Format::DXBC5_SNORM;			// two lines through 1D space, 4x4 blocks, signed normalized

		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:				return Format::DXBC6H_UFLOAT;			// 3-component, 4x4 blocks, unsigned floating-point
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:				return Format::DXBC6H_SFLOAT;			// 3-component, 4x4 blocks, signed floating-point
		case GL_COMPRESSED_RGBA_BPTC_UNORM:						return Format::DXBC7_UNORM;			// 4-component, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:				return Format::DXBC7_SRGB;			// 4-component, 4x4 blocks, sRGB

			//
			// ETC
			//
		case GL_ETC1_RGB8_OES:									return Format::ETC2_R8G8B8_UNORM;	// 3-component ETC1, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_RGB8_ETC2:							return Format::ETC2_R8G8B8_UNORM;	// 3-component ETC2, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return Format::ETC2_R8G8B8A1_UNORM;	// 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA8_ETC2_EAC:						return Format::ETC2_R8G8B8A8_UNORM;	// 4-component ETC2, 4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ETC2:							return Format::ETC2_R8G8B8_SRGB;	// 3-component ETC2, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:		return Format::ETC2_R8G8B8A1_SRGB;	// 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:				return Format::ETC2_R8G8B8A8_SRGB;	// 4-component ETC2, 4x4 blocks, sRGB
#if false
		case GL_COMPRESSED_R11_EAC:								return Format::EAC_R11_UNORM;		// 1-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RG11_EAC:							return Format::EAC_R11G11_UNORM;	// 2-component ETC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_SIGNED_R11_EAC:						return Format::EAC_R11_SNORM;		// 1-component ETC, 4x4 blocks, signed normalized
		case GL_COMPRESSED_SIGNED_RG11_EAC:						return Format::EAC_R11G11_SNORM;	// 2-component ETC, 4x4 blocks, signed normalized
#endif
			//
			// PVRTC
			//
		case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG: return Format::PVRTC1_2BPP_UNORM;			// 3-component PVRTC, 16x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG: return Format::PVRTC1_4BPP_UNORM;			// 3-component PVRTC,  8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: return Format::PVRTC1_2BPP_UNORM;			// 4-component PVRTC, 16x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: return Format::PVRTC1_4BPP_UNORM;			// 4-component PVRTC,  8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG: return Format::PVRTC2_2BPP_UNORM;			// 4-component PVRTC,  8x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG: return Format::PVRTC2_4BPP_UNORM;			// 4-component PVRTC,  4x4 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT: return Format::PVRTC1_2BPP_SRGB;			// 3-component PVRTC, 16x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT: return Format::PVRTC1_4BPP_SRGB;			// 3-component PVRTC,  8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT: return Format::PVRTC1_2BPP_SRGB;	// 4-component PVRTC, 16x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT: return Format::PVRTC1_4BPP_SRGB;	// 4-component PVRTC,  8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG: return Format::PVRTC2_2BPP_SRGB;	// 4-component PVRTC,  8x4 blocks, sRGB
		case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG: return Format::PVRTC2_4BPP_SRGB;	// 4-component PVRTC,  4x4 blocks, sRGB

			//
			// ASTC
			//
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:					return Format::ASTC_4x4_UNORM;		// 4-component ASTC, 4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:					return Format::ASTC_5x4_UNORM;		// 4-component ASTC, 5x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:					return Format::ASTC_5x5_UNORM;		// 4-component ASTC, 5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:					return Format::ASTC_6x5_UNORM;		// 4-component ASTC, 6x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:					return Format::ASTC_6x6_UNORM;		// 4-component ASTC, 6x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:					return Format::ASTC_8x5_UNORM;		// 4-component ASTC, 8x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:					return Format::ASTC_8x6_UNORM;		// 4-component ASTC, 8x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:					return Format::ASTC_8x8_UNORM;		// 4-component ASTC, 8x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:					return Format::ASTC_10x5_UNORM;		// 4-component ASTC, 10x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:					return Format::ASTC_10x6_UNORM;		// 4-component ASTC, 10x6 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:					return Format::ASTC_10x8_UNORM;		// 4-component ASTC, 10x8 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:					return Format::ASTC_10x10_UNORM;	// 4-component ASTC, 10x10 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:					return Format::ASTC_12x10_UNORM;	// 4-component ASTC, 12x10 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:					return Format::ASTC_12x12_UNORM;	// 4-component ASTC, 12x12 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:			return Format::ASTC_4x4_SRGB;		// 4-component ASTC, 4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:			return Format::ASTC_5x4_SRGB;		// 4-component ASTC, 5x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:			return Format::ASTC_5x5_SRGB;		// 4-component ASTC, 5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:			return Format::ASTC_6x5_SRGB;		// 4-component ASTC, 6x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:			return Format::ASTC_6x6_SRGB;		// 4-component ASTC, 6x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:			return Format::ASTC_8x5_SRGB;		// 4-component ASTC, 8x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:			return Format::ASTC_8x6_SRGB;		// 4-component ASTC, 8x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:			return Format::ASTC_8x8_SRGB;		// 4-component ASTC, 8x8 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:			return Format::ASTC_10x5_SRGB;		// 4-component ASTC, 10x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:			return Format::ASTC_10x6_SRGB;		// 4-component ASTC, 10x6 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:			return Format::ASTC_10x8_SRGB;		// 4-component ASTC, 10x8 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:			return Format::ASTC_10x10_SRGB;		// 4-component ASTC, 10x10 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:			return Format::ASTC_12x10_SRGB;		// 4-component ASTC, 12x10 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:			return Format::ASTC_12x12_SRGB;		// 4-component ASTC, 12x12 blocks, sRGB

		case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:					return Format::UNDEFINED;					// 4-component ASTC, 3x3x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:					return Format::UNDEFINED;					// 4-component ASTC, 4x3x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:					return Format::UNDEFINED;					// 4-component ASTC, 4x4x3 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:					return Format::UNDEFINED;					// 4-component ASTC, 4x4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:					return Format::UNDEFINED;					// 4-component ASTC, 5x4x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:					return Format::UNDEFINED;					// 4-component ASTC, 5x5x4 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:					return Format::UNDEFINED;					// 4-component ASTC, 5x5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:					return Format::UNDEFINED;					// 4-component ASTC, 6x5x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:					return Format::UNDEFINED;					// 4-component ASTC, 6x6x5 blocks, unsigned normalized
		case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:					return Format::UNDEFINED;					// 4-component ASTC, 6x6x6 blocks, unsigned normalized

		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:			return Format::UNDEFINED;					// 4-component ASTC, 3x3x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:			return Format::UNDEFINED;					// 4-component ASTC, 4x3x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:			return Format::UNDEFINED;					// 4-component ASTC, 4x4x3 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:			return Format::UNDEFINED;					// 4-component ASTC, 4x4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:			return Format::UNDEFINED;					// 4-component ASTC, 5x4x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:			return Format::UNDEFINED;					// 4-component ASTC, 5x5x4 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:			return Format::UNDEFINED;					// 4-component ASTC, 5x5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:			return Format::UNDEFINED;					// 4-component ASTC, 6x5x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:			return Format::UNDEFINED;					// 4-component ASTC, 6x6x5 blocks, sRGB
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:			return Format::UNDEFINED;					// 4-component ASTC, 6x6x6 blocks, sRGB

			//
			// ATC
			//
		case GL_ATC_RGB_AMD:									return Format::UNDEFINED;					// 3-component, 4x4 blocks, unsigned normalized
		case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:					return Format::UNDEFINED;					// 4-component, 4x4 blocks, unsigned normalized
		case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:				return Format::UNDEFINED;					// 4-component, 4x4 blocks, unsigned normalized

			//
			// Palletized
			//
		case GL_PALETTE4_RGB8_OES:								return Format::UNDEFINED;					// 3-component 8:8:8,   4-bit palette, unsigned normalized
		case GL_PALETTE4_RGBA8_OES:								return Format::UNDEFINED;					// 4-component 8:8:8:8, 4-bit palette, unsigned normalized
		case GL_PALETTE4_R5_G6_B5_OES:							return Format::UNDEFINED;					// 3-component 5:6:5,   4-bit palette, unsigned normalized
		case GL_PALETTE4_RGBA4_OES:								return Format::UNDEFINED;					// 4-component 4:4:4:4, 4-bit palette, unsigned normalized
		case GL_PALETTE4_RGB5_A1_OES:							return Format::UNDEFINED;					// 4-component 5:5:5:1, 4-bit palette, unsigned normalized
		case GL_PALETTE8_RGB8_OES:								return Format::UNDEFINED;					// 3-component 8:8:8,   8-bit palette, unsigned normalized
		case GL_PALETTE8_RGBA8_OES:								return Format::UNDEFINED;					// 4-component 8:8:8:8, 8-bit palette, unsigned normalized
		case GL_PALETTE8_R5_G6_B5_OES:							return Format::UNDEFINED;					// 3-component 5:6:5,   8-bit palette, unsigned normalized
		case GL_PALETTE8_RGBA4_OES:								return Format::UNDEFINED;					// 4-component 4:4:4:4, 8-bit palette, unsigned normalized
		case GL_PALETTE8_RGB5_A1_OES:							return Format::UNDEFINED;					// 4-component 5:5:5:1, 8-bit palette, unsigned normalized

			//
			// Depth/stencil
			//
		case GL_DEPTH_COMPONENT16:								return Format::D16_UNORM;
		case GL_DEPTH_COMPONENT24:								return Format::X8_D24_UNORM;
		case GL_DEPTH_COMPONENT32:								return Format::UNDEFINED;
		case GL_DEPTH_COMPONENT32F:								return Format::D32_SFLOAT;
		case GL_DEPTH_COMPONENT32F_NV:							return Format::D32_SFLOAT;
		case GL_STENCIL_INDEX1:									return Format::UNDEFINED;
		case GL_STENCIL_INDEX4:									return Format::UNDEFINED;
		case GL_STENCIL_INDEX8:									return Format::S8_UINT;
		case GL_STENCIL_INDEX16:								return Format::UNDEFINED;
		case GL_DEPTH24_STENCIL8:								return Format::D24_UNORM_S8_UINT;
		case GL_DEPTH32F_STENCIL8:								return Format::D32_SFLOAT_S8_UINT;
		case GL_DEPTH32F_STENCIL8_NV:							return Format::D32_SFLOAT_S8_UINT;

		default:												return Format::UNDEFINED;
		}
	}

	KtxTextureLoader::~KtxTextureLoader() {

	}

	Ref<Texture> KtxTextureLoader::load(const String& fileName, SamplerInfo samplerInfo) {
		TextureData textureData;
		auto content = FileUtils::readAll(fileName);

		auto result = ktxTexture_CreateFromMemory(content.data(), content.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
		assert(result == KTX_SUCCESS);

		textureData.width = ktxTexture->baseWidth;
		textureData.height = ktxTexture->baseHeight;
		textureData.depth = ktxTexture->baseDepth;
		textureData.mipMapCount = ktxTexture->numLevels;
		textureData.layerCount = ktxTexture->numLayers;
		textureData.faceCount = ktxTexture->numFaces;

		textureData.format = GetFormatFromOpenGLInternalFormat(ktxTexture->glInternalformat);

		ktxTextureData = ktxTexture_GetData(ktxTexture);
		ktxTextureSize = ktxTexture_GetSize(ktxTexture);
		
		Ref<Texture> tex(new Texture());
		if (!tex->create(textureData, samplerInfo)) {
			return nullptr;
		}

		tex->copyData(this);
		return tex;
	}

	void KtxTextureLoader::copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level) {

		ktx_size_t offset;
		KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, level, layer, face, &offset);
		assert(result == KTX_SUCCESS);
		std::memcpy(pDest, ktxTextureData + offset, imageSize);
	}


}