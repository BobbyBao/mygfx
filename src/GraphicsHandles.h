#pragma once
#include "GraphicsDefs.h"
#include "utils/SharedPtr.h"
#include <vector>

#if defined(VK_USE_PLATFORM_METAL_EXT)
#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"
#endif

namespace mygfx {

class HwResource : public HwObject {
public:
    void initState(ResourceState initialState);
    ResourceState getCurrentResourceState(uint32_t subResource = 0xffffffff) const;
    void setCurrentResourceState(ResourceState newState, uint32_t subResource = 0xffffffff);
    void initSubResourceCount(uint32_t subResourceCount);

    ResourceType type = ResourceType::UNKNOWN;
    std::vector<ResourceState> mCurrentStates;
};

class HwBuffer : public HwResource {
public:
    HwBuffer() { type = ResourceType::BUFFER; }

    uint64_t size = 0;
    BufferUsage usage = BufferUsage::NONE;
    MemoryUsage memoryUsage = MemoryUsage::GPU_ONLY;
    uint16_t stride = 0;
    uint16_t extra = 0;
    void* mapped = nullptr;
    uint64_t deviceAddress = 0;
    uint64_t bufferOffset = 0;

    uint64_t count() const
    {
        return stride == 0 ? 0 : size / stride;
    }
};

inline uint64_t BufferInfo::getDeviceAddress() const
{
    return buffer ? buffer->deviceAddress + offset : 0;
}

class HwBufferView : public HwObject {
public:
    inline int index() const { return index_; }

    int index_ = -1;
};

class HwTextureView : public HwObject {
public:
    inline int index() const { return index_; }

    int index_ = -1;
};

class HwTexture : public HwResource {
public:
    HwTexture() { type = ResourceType::IMAGE; }

    uint16_t width = 0, height = 0;
    uint16_t depth = 1;
    uint16_t layerCount = 1;
    uint16_t faceCount = 1;
    uint16_t mipLevels = 1;
    Format format = Format::UNDEFINED;
    SamplerType samplerType = SamplerType::SAMPLER_2D;

    Ref<HwTextureView> mSRV;
    Ref<HwTextureView> mRTV;
    Ref<HwTextureView> mDSV;

    HwTextureView* getSRV() const { return mSRV.get(); }
    HwTextureView* getRTV() const { return mRTV.get(); }
    HwTextureView* getDSV() const { return mDSV.get(); }

    bool isCubemap() const
    {
        return faceCount == 6;
    }

    static Ref<HwTexture> Black;
    static Ref<HwTexture> White;
    static Ref<HwTexture> Magenta;
};

struct SamplerHandle : public HwObject {
    SamplerInfo samplerInfo;
    uint16_t index = 0;

    static Ref<SamplerHandle> NearestRepeat;
    static Ref<SamplerHandle> NearestClampToEdge;
    static Ref<SamplerHandle> NearestClampToBorder;
    static Ref<SamplerHandle> LinearRepeat;
    static Ref<SamplerHandle> LinearClampToEdge;
    static Ref<SamplerHandle> LinearClampToBorder;
    static Ref<SamplerHandle> Shadow;

    static void init();
    static void shutdown();
};

class ShaderResourceInfo;

class HwShaderModule : public HwObject {
public:
    std::vector<Ref<ShaderResourceInfo>> shaderResourceInfo;
    Ref<ShaderResourceInfo> materialUniforms;
};

class HwVertexInput : public HwObject {
public:
};

class HwDescriptorSet : public HwObject {
public:
};

class HwProgram : public HwObject {
public:
    ShaderResourceInfo* getShaderResource(const String& name);
    HwDescriptorSet* getDescriptorSet(uint32_t index);
    HwDescriptorSet* createDescriptorSet(uint32_t index);
    Ref<HwVertexInput> vertexInput;
};

struct RenderTargetDesc {
    uint32_t width, height;

    std::vector<Ref<HwTextureView>> colorAttachments;
    Ref<HwTextureView> depthAttachment = nullptr;
};

class HwRenderTarget : public HwObject {
public:
    uint32_t width, height;
    uint32_t currentIndex = 0;
    bool isSwapchain = false;
};

struct RenderPassInfo {
    Viewport viewport {};
    uint32_t passID = 0;
    TargetBufferFlags clearFlags = TargetBufferFlags::NONE;
    TargetBufferFlags loadFlags = TargetBufferFlags::NONE;
    TargetBufferFlags storeFlags = TargetBufferFlags::ALL;
    float clearColor[4] = { 0 };
    float clearDepth = 0.0;
    uint32_t clearStencil = 0;
};

struct SwapChainDesc {
    uint32_t width = 1280;
    uint32_t height = 720;
    Format colorFormat = Format::R8G8B8A8_SNORM;
    Format depthFormat = Format::UNDEFINED;
    bool fullscreen = false;
    bool vsync = false;
    void* windowInstance = nullptr;
#if defined(VK_USE_PLATFORM_METAL_EXT)
    CAMetalLayer* window = nullptr;
#else
    void* window = nullptr;
#endif
    void* surface = nullptr;
};

class HwSwapchain : public HwObject {
public:
    SwapChainDesc desc;
    Ref<HwRenderTarget> renderTarget;
};

struct DrawPrimitiveCommand {

    union {
        uint32_t indexCount;
        uint32_t vertexCount;
    };

    union {
        uint32_t firstIndex = 0;
        uint32_t firstVertex;
    };

    int32_t vertexOffset = 0;
};

class VertexData : public RefCounted {
public:
    std::vector<Ref<HwBuffer>> vertexBuffers;
    Ref<HwBuffer> indexBuffer;
};

class HwRenderPrimitive : public HwObject {
public:
    HwRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive);

    DrawPrimitiveCommand drawArgs;

protected:
    Ref<VertexData> mGeometry;
};

}
