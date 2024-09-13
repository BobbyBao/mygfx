#pragma once
#include "Format.h"
#include "GraphicsConsts.h"
#include "GraphicsFwd.h"
#include "utils/BitmaskEnum.h"
#include <stdint.h>

namespace mygfx {

enum class BufferUsage : uint16_t {
    NONE = 0,
    VERTEX = (1 << 0),
    INDEX = (1 << 1),
    UNIFORM = (1 << 2),
    STORAGE = (1 << 3),
    UNIFORM_TEXEL = (1 << 4),
    STORAGE_TEXEL = (1 << 5),
    SHADER_DEVICE_ADDRESS = (1 << 6),
};

enum class MemoryUsage : uint8_t {
    UNKNOWN = 0,
    GPU_ONLY = 1,
    CPU_ONLY = 2,
    CPU_TO_GPU = 3,
    GPU_TO_CPU = 4,
    CPU_COPY = 5,
    GPU_LAZILY_ALLOCATED = 6
};

enum class TextureUsage : uint16_t {
    NONE,
    TRANSFER_SRC = 0x00000001,
    TRANSFER_DST = 0x00000002,
    SAMPLED = 0x00000004,
    STORAGE = 0x00000008,
    COLOR_ATTACHMENT = 0x00000010,
    DEPTH_STENCIL_ATTACHMENT = 0x00000020,
    TRANSIENT_ATTACHMENT = 0x00000040,
    INPUTATT_ACHMENT = 0x00000080,
    FRAGMENT_SHADINGRATE_ATTACHMENT = 0x00000100,
    FRAGMENT_DENSITY_MAP = 0x00000200,
    VIDEO_DECODE_DST = 0x00000400,
    VIDEO_DECODE_SRC = 0x00000800,
    VIDEO_DECODE_DPB = 0x00001000,
};

enum class IndexType : uint8_t {
    UINT16 = 0,
    UINT32 = 1,
};

enum class VertexAttribute : uint16_t {
    NONE = 0,
    POSITION = (1 << 0),
    NORMAL = (1 << 1),
    TANGENTS = (1 << 2),
    COLOR = (1 << 3),
    UV_0 = (1 << 4),
    UV_1 = (1 << 5),
    BONE_INDICES = (1 << 6),
    BONE_WEIGHTS = (1 << 7),

    CUSTOM_0 = (1 << 8),
    CUSTOM_1 = (1 << 9),
    CUSTOM_2 = (1 << 10),
    CUSTOM_3 = (1 << 11),
    CUSTOM_4 = (1 << 12),
    CUSTOM_5 = (1 << 13),
    CUSTOM_6 = (1 << 14),
    CUSTOM_7 = (1 << 15),

    ALL = 0xffff
};

enum class CullMode : uint8_t {
    NONE = 0,
    FRONT = 0x00000001,
    BACK = 0x00000002,
    FRONT_AND_BACK = 0x00000003,
};

enum class FrontFace : uint8_t {
    COUNTER_CLOCKWISE = 0,
    CLOCKWISE = 1,
};

enum class BlendFactor : uint32_t {
    ZERO = 0,
    ONE = 1,
    SRC_COLOR = 2,
    ONE_MINUS_SRC_COLOR = 3,
    DST_COLOR = 4,
    ONE_MINUS_DST_COLOR = 5,
    SRC_ALPHA = 6,
    ONE_MINUS_SRC_ALPHA = 7,
    DST_ALPHA = 8,
    ONE_MINUS_DST_ALPHA = 9,
    CONSTANT_COLOR = 10,
    ONE_MINUS_CONSTANT_COLOR = 11,
    CONSTANT_ALPHA = 12,
    ONE_MINUS_CONSTANT_ALPHA = 13,
    SRC_ALPHA_SATURATE = 14,
    SRC1_COLOR = 15,
    ONE_MINUS_SRC1_COLOR = 16,
    SRC1_ALPHA = 17,
    ONE_MINUS_SRC1_ALPHA = 18,
};

enum class BlendOp : uint32_t {
    ADD = 0,
    SUBTRACT = 1,
    REVERSE_SUBTRACT = 2,
    MIN = 3,
    MAX = 4,
};

enum class CompareOp : uint8_t {
    NEVER = 0,
    LESS = 1,
    EQUAL = 2,
    LESS_OR_EQUAL = 3,
    GREATER = 4,
    NOT_EQUAL = 5,
    GREATER_OR_EQUAL = 6,
    ALWAYS = 7,
};

enum class PrimitiveTopology : uint8_t {
    POINT_LIST = 0,
    LINE_LIST = 1,
    LINE_STRIP = 2,
    TRIANGLE_LIST = 3,
    TRIANGLE_STRIP = 4,
    TRIANGLE_FAN = 5,
    LINE_LIST_WITH_ADJACENCY = 6,
    LINE_STRIP_WITH_ADJACENCY = 7,
    TRIANGLE_LIST_WITH_ADJACENCY = 8,
    TRIANGLE_STRIP_WITH_ADJACENCY = 9,
    PATCH_LIST = 10,
};

enum class PolygonMode : uint8_t {
    FILL = 0,
    LINE = 1,
    POINT = 2,
};

enum class StencilOp : uint8_t {
    KEEP = 0,
    ZERO = 1,
    REPLACE = 2,
    INCREMENT_AND_CLAMP = 3,
    DECREMENT_AND_CLAMP = 4,
    INVERT = 5,
    INCREMENT_AND_WRAP = 6,
    DECREMENT_AND_WRAP = 7,
};

enum class LogicOp : uint8_t {
    CLEAR = 0,
    AND = 1,
    AND_REVERSE = 2,
    COPY = 3,
    AND_INVERTED = 4,
    NO_OP = 5,
    XOR = 6,
    OR = 7,
    NOR = 8,
    EQUIVALENT = 9,
    INVERT = 10,
    OR_REVERSE = 11,
    COPY_INVERTED = 12,
    OR_INVERTED = 13,
    NAND = 14,
    SET = 15,
};

enum class SampleCount : uint8_t {
    SAMPLE_1 = 0x00000001,
    SAMPLE_2 = 0x00000002,
    SAMPLE_4 = 0x00000004,
    SAMPLE_8 = 0x00000008,
    SAMPLE_16 = 0x00000010,
    SAMPLE_32 = 0x00000020,
    SAMPLE_64 = 0x00000040,
};

enum SamplerType : uint8_t {
    SAMPLER_1D = 0,
    SAMPLER_2D = 1,
    SAMPLER_3D = 2,
    SAMPLER_CUBE = 3,
    SAMPLER_1D_ARRAY = 4,
    SAMPLER_2D_ARRAY = 5,
    SAMPLER_CUBE_ARRAY = 6,
    COUNT,
};

enum class ColorComponent : uint32_t {
    R = 0x00000001,
    G = 0x00000002,
    B = 0x00000004,
    A = 0x00000008,
    RGB = R | G | B,
    RGBA = R | G | B | A,
};

enum class Filter : uint8_t {
    NEAREST = 0,
    LINEAR = 1,
};

enum class SamplerAddressMode : uint8_t {
    REPEAT = 0,
    MIRRORED_REPEAT = 1,
    CLAMP_TO_EDGE = 2,
    CLAMP_TO_BORDER = 3,
    MIRROR_CLAMP_TO_EDGE = 4,
};

enum class BorderColor : uint8_t {
    FLOAT_TRANSPARENT_BLACK = 0,
    INT_TRANSPARENT_BLACK = 1,
    FLOAT_OPAQUE_BLACK = 2,
    INT_OPAQUE_BLACK = 3,
    FLOAT_OPAQUE_WHITE = 4,
    INT_OPAQUE_WHITE = 5,
};

enum class DescriptorType : uint8_t {
    SAMPLER = 0,
    COMBINED_IMAGE_SAMPLER = 1,
    SAMPLED_IMAGE = 2,
    STORAGE_IMAGE = 3,
    UNIFORM_TEXEL_BUFFER = 4,
    STORAGE_TEXEL_BUFFER = 5,
    UNIFORM_BUFFER = 6,
    STORAGE_BUFFER = 7,
    UNIFORM_BUFFER_DYNAMIC = 8,
    STORAGE_BUFFER_DYNAMIC = 9,
    INPUT_ATTACHMENT = 10,
};

enum class ShaderStage : uint32_t {
    None = 0,
    VERTEX = 0x00000001,
    TESSELLATION_CONTROL = 0x00000002,
    TESSELLATION_EVALUATION = 0x00000004,
    GEOMETRY = 0x00000008,
    FRAGMENT = 0x00000010,
    COMPUTE = 0x00000020,
    ALL_GRAPHICS = 0x0000001F,
    ALL = 0x7FFFFFFF,
    RAYGEN = 0x00000100,
    ANYHIT = 0x00000200,
    CLOSEST_HIT = 0x00000400,
    MISS = 0x00000800,
    INTERSECTION = 0x00001000,
    CALLABLE = 0x00002000,
    TASK = 0x00000040,
    MESH = 0x00000080,
};

enum class ShaderSourceType : uint8_t {
    GLSL,
    HLSL,
};

enum class ShaderCodeType : uint8_t {
    BINARY = 0,
    SPIRV = 1,
};

enum class ProgramType : uint8_t {
    GRAPHICS,
    COMPUTE,
    RAYTRACING
};

enum class TextureCubemapFace : uint8_t {
    // don't change the enums values
    POSITIVE_X = 0, //!< +x face
    NEGATIVE_X = 1, //!< -x face
    POSITIVE_Y = 2, //!< +y face
    NEGATIVE_Y = 3, //!< -y face
    POSITIVE_Z = 4, //!< +z face
    NEGATIVE_Z = 5, //!< -z face
};

enum class TargetBufferFlags : uint32_t {
    NONE = 0x0u, //!< No buffer selected.
    COLOR_0 = 0x00000001u, //!< Color buffer selected.
    COLOR_1 = 0x00000002u, //!< Color buffer selected.
    COLOR_2 = 0x00000004u, //!< Color buffer selected.
    COLOR_3 = 0x00000008u, //!< Color buffer selected.
    COLOR_4 = 0x00000010u, //!< Color buffer selected.
    COLOR_5 = 0x00000020u, //!< Color buffer selected.
    COLOR_6 = 0x00000040u, //!< Color buffer selected.
    COLOR_7 = 0x00000080u, //!< Color buffer selected.

    COLOR = COLOR_0, //!< \deprecated
    COLOR_ALL = COLOR_0 | COLOR_1 | COLOR_2 | COLOR_3 | COLOR_4 | COLOR_5 | COLOR_6 | COLOR_7,
    DEPTH = 0x10000000u, //!< Depth buffer selected.
    STENCIL = 0x20000000u, //!< Stencil buffer selected.
    DEPTH_AND_STENCIL = DEPTH | STENCIL, //!< depth and stencil buffer selected.
    ALL = COLOR_ALL | DEPTH | STENCIL //!< Color, depth and stencil buffer selected.
};

inline constexpr TargetBufferFlags getTargetBufferFlagsAt(size_t index) noexcept
{
    if (index == 0u)
        return TargetBufferFlags::COLOR_0;
    if (index == 1u)
        return TargetBufferFlags::COLOR_1;
    if (index == 2u)
        return TargetBufferFlags::COLOR_2;
    if (index == 3u)
        return TargetBufferFlags::COLOR_3;
    if (index == 4u)
        return TargetBufferFlags::COLOR_4;
    if (index == 5u)
        return TargetBufferFlags::COLOR_5;
    if (index == 6u)
        return TargetBufferFlags::COLOR_6;
    if (index == 7u)
        return TargetBufferFlags::COLOR_7;
    if (index == 8u)
        return TargetBufferFlags::DEPTH;
    if (index == 9u)
        return TargetBufferFlags::STENCIL;
    return TargetBufferFlags::NONE;
}

class HwBuffer;
struct TextureData;
class TextureDataProvider;
class HwTexture;
class DefineList;
struct SamplerHandle;

struct BufferInfo {
    HwBuffer* buffer = nullptr;
    uint32_t offset = 0;
    uint32_t range = 0;

    uint64_t getDeviceAddress() const;
};

struct SamplerInfo {
    Filter magFilter : 1 = Filter::LINEAR;
    Filter minFilter : 1 = Filter::LINEAR;
    Filter mipmapMode : 1 = Filter::LINEAR;
    SamplerAddressMode addressModeU : 3 = SamplerAddressMode::REPEAT;

    SamplerAddressMode addressModeV : 3 = SamplerAddressMode::REPEAT;
    SamplerAddressMode addressModeW : 3 = SamplerAddressMode::REPEAT;
    bool compareEnable : 1 = false;

    CompareOp compareOp : 3 = CompareOp::NEVER;
    BorderColor borderColor : 3 = BorderColor::FLOAT_TRANSPARENT_BLACK;
    bool unnormalizedCoordinates : 1 = false;
    bool anisotropyEnable : 1 = false;

    bool srgb : 1 = false;
    // uint8_t reserve : 7;

    constexpr static SamplerInfo create(Filter filter, SamplerAddressMode addressMode, bool srgb = false)
    {
        SamplerInfo ret {};
        ret.magFilter = filter;
        ret.minFilter = filter;
        ret.mipmapMode = filter;
        ret.addressModeU = addressMode;
        ret.addressModeV = addressMode;
        ret.addressModeW = addressMode;
        ret.srgb = srgb;
        return ret;
    }

    bool operator==(const SamplerInfo& other) const
    {
        return std::memcmp(this, &other, sizeof(SamplerInfo)) == 0;
    }
};

static_assert(sizeof(SamplerInfo) == 4);

enum class ResourceState : uint32_t {
    COMMON_RESOURCE = 0x0, ///< Common resource state.
    VERTEX_BUFFER_RESOURCE = 0x1 << 0, ///< Vertex buffer resource state.
    CONSTANT_BUFFER_RESOURCE = 0x1 << 1, ///< Constant buffer resource state.
    INDEX_BUFFER_RESOURCE = 0x1 << 2, ///< Index buffer resource state.
    RENDER_TARGET_RESOURCE = 0x1 << 3, ///< Render target resource state.
    UNORDERED_ACCESS = 0x1 << 4, ///< Unordered access resource state.
    DEPTH_WRITE = 0x1 << 5, ///< Depth write resource state.
    DEPTH_READ = 0x1 << 6, ///< Depth read resource state.
    NONPIXEL_SHADER_RESOURCE = 0x1 << 7, ///< Non-pixel shader resource state.
    PIXEL_SHADER_RESOURCE = 0x1 << 8, ///< Pixel shader resource state.
    INDIRECT_ARGUMENT = 0x1 << 9, ///< Indirect argument resource state.
    COPY_DEST = 0x1 << 10, ///< Copy destination resource state.
    COPY_SOURCE = 0x1 << 11, ///< Copy source resource state.
    RESOLVE_DEST = 0x1 << 12, ///< Resolve destination resource state.
    RESOLVE_SOURCE = 0x1 << 13, ///< Resolve source resource state.
    RT_ACCELERATION_STRUCT = 0x1 << 14, ///< Ray tracing acceleration structure resource state.
    SHADING_RATE_SOURCE = 0x1 << 15, ///< Shading rate source resource state.
    GENERICREAD = 0x1 << 16, ///< Generic read resource state.
    PRESENT = 0x1 << 17, ///< Present resource state.

    // Special cases
    DEPTH_SHADER_RESOURCE = DEPTH_READ | NONPIXEL_SHADER_RESOURCE | PIXEL_SHADER_RESOURCE, ///< Depth shader resource state.
    SHADER_RESOURCE = NONPIXEL_SHADER_RESOURCE | PIXEL_SHADER_RESOURCE, ///< Shader resource (general) state
    UNDEFINED = (uint32_t)-1,
};

struct DescriptorSetLayoutBinding {
    uint32_t binding;
    DescriptorType descriptorType;
    uint32_t descriptorCount;
    ShaderStage stageFlags;
    void* pImmutableSamplers;
    String name;
};

enum class BarrierType {
    TRANSITION,
    ALIASING,
    UAV,
};

class HwResource;

enum class ResourceType {
    UNKNOWN,
    IMAGE,
    BUFFER,
};

struct Barrier {
    BarrierType type;
    const HwResource* pResource;
    ResourceState sourceState;
    ResourceState destState;
    uint32_t subResource = 0xffffffff;

    static Barrier Transition(const HwResource* pRes, ResourceState srcState, ResourceState dstState, uint32_t subResource = 0xffffffff)
    {
        Barrier barrier = { BarrierType::TRANSITION, pRes, srcState, dstState, subResource };
        return barrier;
    }

    static Barrier UAV(const HwResource* pRes)
    {
        Barrier barrier = { BarrierType::UAV, pRes, ResourceState::UNORDERED_ACCESS, ResourceState::UNORDERED_ACCESS };
        return barrier;
    }
};

struct Viewport {
    int32_t left = 0;
    int32_t top = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    float nearZ = 0.0f;
    float farZ = 1.0f;

    int32_t right() const noexcept { return left + int32_t(width); }
    int32_t bottom() const noexcept { return top + int32_t(height); }
};

struct Extent2D {
    uint32_t width;
    uint32_t height;
};

struct Extent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct Offset2D {
    int32_t x;
    int32_t y;
};

struct Offset3D {
    int32_t x;
    int32_t y;
    int32_t z;
};

}

template <>
struct utils::EnableBitMaskOperators<mygfx::BufferUsage>
    : public std::true_type { };

template <>
struct utils::EnableBitMaskOperators<mygfx::TextureUsage>
    : public std::true_type { };

template <>
struct utils::EnableBitMaskOperators<mygfx::VertexAttribute>
    : public std::true_type { };

template <>
struct utils::EnableBitMaskOperators<mygfx::ColorComponent>
    : public std::true_type { };

template <>
struct utils::EnableBitMaskOperators<mygfx::ShaderStage>
    : public std::true_type { };

template <>
struct utils::EnableBitMaskOperators<mygfx::DescriptorType>
    : public std::true_type { };

template <>
struct utils::EnableBitMaskOperators<mygfx::TargetBufferFlags>
    : public std::true_type { };

template <>
struct utils::EnableBitMaskOperators<mygfx::ResourceState>
    : public std::true_type { };
