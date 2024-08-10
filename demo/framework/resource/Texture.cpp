#include "Texture.h"
#include "GraphicsApi.h"
#define STB_IMAGE_IMPLEMENTATION
#include "utils/FileUtils.h"
#include <stb/stb_image.h>

#include "TextureLoader.h"

namespace mygfx {

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
    White = Texture::createByColor("White", float4 { 1.0f, 1.0f, 1.0f, 1.0f });
    Black = Texture::createByColor("Black", float4 { 0.0f, 0.0f, 0.0f, 1.0f });
    Gray = Texture::createByColor("Gray", float4 { 0.5f, 0.5f, 0.5f, 1.0f });
    Red = Texture::createByColor("Red", float4 { 1.0f, 0.0f, 0.0f, 1.0f });
    Green = Texture::createByColor("Green", float4 { 0.0f, 1.0f, 0.0f, 1.0f });
    Blue = Texture::createByColor("Blue", float4 { 0.0f, 0.0f, 1.0f, 1.0f });
    Yellow = Texture::createByColor("Yellow", float4 { 1.0f, 1.0f, 0.0f, 1.0f });
    Cyan = Texture::createByColor("Cyan", float4 { 0.0f, 1.0f, 1.0f, 1.0f });
    Magenta = Texture::createByColor("Magenta", float4 { 1.0f, 0.0f, 1.0f, 1.0f });
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

bool Texture::create(const TextureData& textureData, SamplerInfo samplerInfo)
{
    mSamplerInfo = samplerInfo;
    mTextureData = textureData;
    return onCreate();
}

bool Texture::onCreate()
{
    mHwTexture = gfxApi().createTexture(mTextureData, mSamplerInfo);
    return mHwTexture != nullptr;
}

void Texture::copyData(TextureDataProvider* dataProvider)
{
    gfxApi().copyData(mHwTexture, dataProvider);
}

void Texture::setData(uint32_t level,
    uint32_t xoffset,
    uint32_t yoffset,
    uint32_t zoffset,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    const void* data,
    size_t size)
{

    gfxApi().updateTexture(mHwTexture, level, xoffset,
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
    MemoryBlock memoryBlock((uint8_t*)&c, 4);
    ;
    auto textureData = TextureData::Texture2D(1, 1, Format::R8G8B8A8_UNORM, memoryBlock);
    textureData.usage = TextureUsage::SAMPLED | TextureUsage::TRANSFER_DST;
    textureData.name = name;
    auto tex = createFromData(textureData);
    return tex;
}

Vector<Ref<Texture>> Texture::createRandomColorTextures(int count)
{
    Vector<Ref<Texture>> ret;
    for (int i = 0; i < count; i++) {
        auto tex = Texture::createByColor("", vec4 { glm::linearRand<float>(0, 1.0f), glm::linearRand<float>(0, 1.0f), glm::linearRand<float>(0, 1.0f), 1.0f });
        ret.push_back(tex);
    }
    return ret;
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
    if (fileName.ends_with(".ktx") || fileName.ends_with(".ktx2")) {
        KtxTextureLoader dataProvider;
        return dataProvider.load(fileName, samplerInfo);
    } else if (fileName.ends_with(".dds")) {
        DDSTextureLoader dataProvider;
        return dataProvider.load(fileName, samplerInfo);
    }

    StbTextureLoader dataProvider;
    return dataProvider.load(fileName, samplerInfo);
}

Ref<Texture> Texture::createFromData(const Span<uint8_t>& content, const String& type, SamplerInfo samplerInfo)
{

    if (type.ends_with(".ktx")) {
        KtxTextureLoader dataProvider;
        return dataProvider.onLoad(content, samplerInfo);
    } else if (type.ends_with(".dds")) {
        DDSTextureLoader dataProvider;
        return dataProvider.onLoad(content, samplerInfo);
    }

    StbTextureLoader dataProvider;
    return dataProvider.onLoad(content, samplerInfo);
}

Ref<Texture> Texture::createRenderTarget(uint16_t width, uint16_t height, Format format, TextureUsage usage, SampleCount msaa)
{
    auto textureData = TextureData::Texture2D(width, height, format);
    textureData.sampleCount = msaa;
    textureData.usage = TextureUsage::COLOR_ATTACHMENT | usage;
    return createFromData(textureData, SamplerInfo::create(Filter::NEAREST, SamplerAddressMode::CLAMP_TO_EDGE));
}

Ref<Texture> Texture::createDepthStencil(uint16_t width, uint16_t height, Format format, TextureUsage usage, bool isShadowMap, SampleCount msaa)
{
    auto textureData = TextureData::Texture2D(width, height, format);
    textureData.sampleCount = msaa;
    textureData.usage = TextureUsage::DEPTH_STENCIL_ATTACHMENT | usage;

    SamplerInfo samplerInfo = SamplerInfo::create(Filter::NEAREST, SamplerAddressMode::CLAMP_TO_EDGE);

    if (isShadowMap) {
        textureData.usage |= TextureUsage::SAMPLED;
        samplerInfo.compareEnable = true;
        samplerInfo.compareOp = CompareOp::LESS_OR_EQUAL;
    }

    return createFromData(textureData, samplerInfo);
}

StbTextureLoader::~StbTextureLoader()
{
    if (mData)
        stbi_image_free(mData);
}

Ref<Texture> StbTextureLoader::onLoad(const Span<uint8_t>& content, SamplerInfo samplerInfo)
{
    int32_t width, height, channels;
    Format format = Format::R8G8B8A8_UNORM;
    mIsHdr = stbi_is_hdr_from_memory(content.data(), (int)content.size());

    if (mIsHdr) {
        mHdrData = stbi_loadf_from_memory(content.data(), (int)content.size(), &width, &height, &channels, STBI_rgb_alpha);
        format = Format::R32G32B32A32_SFLOAT;
    } else {
        mData = (char*)stbi_load_from_memory(content.data(), (int)content.size(), &width, &height, &channels, STBI_rgb_alpha);
    }

    if (!mData) {
        return nullptr;
    }

    uint32_t mipCount = Texture::maxLevelCount(width, height);

    TextureData textureData;
    textureData.layerCount = 1;
    textureData.width = width;
    textureData.height = height;
    textureData.depth = 1;
    textureData.mipMapCount = mipCount;
    textureData.format = format;

    if (samplerInfo.srgb) {
        textureData.format = setFormatGamma(textureData.format, true);
    }

    Ref<Texture> tex(new Texture());
    if (!tex->create(textureData, samplerInfo)) {
        return nullptr;
    }

    tex->copyData(this);
    return tex;
}

void StbTextureLoader::copyPixels(void* pDest, uint32_t imageSize, uint32_t width, uint32_t height, uint32_t layer, uint32_t face, uint32_t level)
{

    memcpy((char*)pDest, mData, imageSize);
    mipImage(width, height);
}

void StbTextureLoader::mipImage(uint32_t width, uint32_t height)
{
    // compute mip so next call gets the lower mip
    int offsetsX[] = { 0, 1, 0, 1 };
    int offsetsY[] = { 0, 0, 1, 1 };

    uint32_t* pImg = (uint32_t*)mData;

#define GetByte(color, component) (((color) >> (8 * (component))) & 0xff)
#define GetColor(ptr, x, y) (ptr[(x) + (y) * width])
#define SetColor(ptr, x, y, col) ptr[(x) + (y) * width / 2] = col;

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

}