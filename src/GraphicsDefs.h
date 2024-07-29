#pragma once
#include <stdint.h>
#include "utils/BitmaskEnum.h"
#include "GraphicsConsts.h"
#include <memory>
#include "Format.h"
#include <string>
#include <span>
#include <vector>

namespace utils {

	template <class T>
	class SharedPtr;
	
	template <class T>
	using Ref = SharedPtr<T>;

}

namespace mygfx {
	
	using namespace utils;

	using String = std::string;

    template<typename T>
    using Span = std::span<T>;
	
	template <class T>
	using Vector = std::vector<T>;

	using ByteArray = std::vector<uint8_t>;

	enum class BufferUsage : uint16_t {
		None = 0,
		Vertex = (1 << 0),
		Index = (1 << 1),
		Uniform = (1 << 2),
		Storage = (1 << 3),
		UniformTexel = (1 << 4),
		StorageTexel = (1 << 5),
		ShaderDeviceAddress = (1 << 6),
	};
	
	enum class MemoryUsage : uint8_t {
		Unknown = 0,
		GpuOnly = 1,
		CpuOnly = 2,
		CpuToGpu = 3,
		GpuToCpu = 4,
		CpuCopy = 5,
		GpuLazilyAllocated = 6
	};

	enum class TextureUsage : uint16_t {
		None,
		TransferSrc = 0x00000001,
		TransferDst = 0x00000002,
		Sampled = 0x00000004,
		Storage = 0x00000008,
		ColorAttachment = 0x00000010,
		DepthStencilAttachment = 0x00000020,
		TransientAttachment = 0x00000040,
		InputAttachment = 0x00000080,
		FragmentShadingRateAttachment = 0x00000100,
		FragmentDensityMap = 0x00000200,
		VideoDecodeDst = 0x00000400,
		VideoDecodeSrc = 0x00000800,
		VideoDecodeDpb = 0x00001000,
	};

	enum class IndexType : uint8_t {
		UInt16 = 0,
		UInt32 = 1,
	};

	enum class VertexAttribute : uint16_t
	{
		None = 0,
		Position = (1 << 0),
		Normal = (1 << 1),
		Tangents = (1 << 2),
		Color = (1 << 3),  
		UV0 = (1 << 4), 
		UV1 = (1 << 5), 
		BoneIndices = (1 << 6),
		BoneWeights = (1 << 7),

		Custom1 = (1 << 8),
		Custom2 = (1 << 9),
		Custom3 = (1 << 10),
		Custom4 = (1 << 11),
		Custom5 = (1 << 12),
		Custom6 = (1 << 13),
		Custom7 = (1 << 14),
		Custom8 = (1 << 15),

		All = 0xffff
	};

	enum class CullMode : uint8_t {
		None = 0,
		Front = 0x00000001,
		Back = 0x00000002,
		FrontAndBack = 0x00000003,
	};
	
	enum class FrontFace : uint8_t {
		CounterClockwise = 0,
		Clockwise = 1,
	};

	enum class BlendFactor : uint32_t {
		Zero = 0,
		One = 1,
		SrcColor = 2,
		OneMinusSrcColor = 3,
		DstColor = 4,
		OneMinusDstColor = 5,
		SrcAlpha = 6,
		OneMinusSrcAlpha = 7,
		DstAlpha = 8,
		OneMinusDstAlpha = 9,
		ConstantColor = 10,
		OneMinusConstantColor = 11,
		ConstantAlpha = 12,
		OneMinusConstantAlpha = 13,
		SrcAlphaSaturate = 14,
		Src1Color = 15,
		OneMinusSrc1Color = 16,
		Src1Alpha = 17,
		OneMinusSrc1Alpha = 18,
	};

	enum class BlendOp : uint32_t {
		Add = 0,
		Subtract = 1,
		ReverseSubtract = 2,
		Min = 3,
		Max = 4,
	};

	enum class CompareOp : uint8_t {
		Never = 0,
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		Always = 7,
	};

	enum class PrimitiveTopology : uint8_t {
		PointList = 0,
		LineList = 1,
		LineStrip = 2,
		TriangleList = 3,
		TriangleStrip = 4,
		TriangleFan = 5,
		LineListWithAdjacency = 6,
		LineStripWithAdjacency = 7,
		TriangleListWithAdjacency = 8,
		TriangleStripWithAdjacency = 9,
		PatchList = 10,
	};

	enum class PolygonMode : uint8_t {
		Fill = 0,
		Line = 1,
		Point = 2,
	}; 

	enum class StencilOp : uint8_t {
		Keep = 0,
		Zero = 1,
		Replace = 2,
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
		_1 = 0x00000001,
		_2 = 0x00000002,
		_4 = 0x00000004,
		_8 = 0x00000008,
		_16 = 0x00000010,
		_32 = 0x00000020,
		_64 = 0x00000040,
	};

	enum SamplerType : uint8_t
	{
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
		RGB = R|G|B,
		RGBA = R|G|B|A,
	};
	

	enum class Filter : uint8_t {
		Nearest = 0,
		Linear = 1,
	};

	enum class SamplerAddressMode : uint8_t {
		Repeat = 0,
		MirroredRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3,
		MirrorClampToEdge = 4,
	};

	enum class BorderColor : uint8_t {
		FloatTransparentBlack = 0,
		IntTransparentBlack = 1,
		FloatOpaqueBlack = 2,
		IntOpaqueBlack = 3,
		FloatOpaqueWhite = 4,
		IntOpaqueWhite = 5,
	};

	enum class DescriptorType : uint8_t {
	    Sampler = 0,
		CombinedImageSampler = 1,
		SampledImage = 2,
		StorageImage = 3,
		UniformTexelBuffer = 4,
		StorageTexelBuffer = 5,
		UniformBuffer = 6,
		StorageBuffer = 7,
		UniformBufferDynamic = 8,
		StorageBufferDynamic = 9,
		InputAttachment = 10,
	};

	enum class ShaderStage : uint32_t
	{
		None = 0,
		Vertex = 0x00000001,
		TessellationControl = 0x00000002,
		TessellationEvaluation = 0x00000004,
		Geometry = 0x00000008,
		Fragment = 0x00000010,
		Compute = 0x00000020,
		AllGraphics = 0x0000001F,
		All = 0x7FFFFFFF,
		Raygen = 0x00000100,
		AnyHit = 0x00000200,
		ClosestHit = 0x00000400,
		Miss = 0x00000800,
		Intersection = 0x00001000,
		Callable = 0x00002000,
		Task = 0x00000040,
		Mesh = 0x00000080,
	};

	enum class ShaderSourceType : uint8_t
	{
		GLSL,
		HLSL,
	};

	enum class ShaderCodeType : uint8_t {
		Binary = 0,
		Spirv = 1,
	};

	enum class ProgramType : uint8_t {
		Graphics,
		Compute,
		Raytracing
	};

	enum class TextureCubemapFace : uint8_t {
		// don't change the enums values
		PositiveX = 0, //!< +x face
		NegativeX = 1, //!< -x face
		PositiveY = 2, //!< +y face
		NegativeY = 3, //!< -y face
		PositiveZ = 4, //!< +z face
		NegativeZ = 5, //!< -z face
	};

	enum class TargetBufferFlags : uint32_t {
		NONE = 0x0u,                            //!< No buffer selected.
		COLOR0 = 0x00000001u,                   //!< Color buffer selected.
		COLOR1 = 0x00000002u,                   //!< Color buffer selected.
		COLOR2 = 0x00000004u,                   //!< Color buffer selected.
		COLOR3 = 0x00000008u,                   //!< Color buffer selected.
		COLOR4 = 0x00000010u,                   //!< Color buffer selected.
		COLOR5 = 0x00000020u,                   //!< Color buffer selected.
		COLOR6 = 0x00000040u,                   //!< Color buffer selected.
		COLOR7 = 0x00000080u,                   //!< Color buffer selected.

		COLOR = COLOR0,                         //!< \deprecated
		COLOR_ALL = COLOR0 | COLOR1 | COLOR2 | COLOR3 | COLOR4 | COLOR5 | COLOR6 | COLOR7,
		DEPTH   = 0x10000000u,                  //!< Depth buffer selected.
		STENCIL = 0x20000000u,                  //!< Stencil buffer selected.
		DEPTH_AND_STENCIL = DEPTH | STENCIL,    //!< depth and stencil buffer selected.
		ALL = COLOR_ALL | DEPTH | STENCIL       //!< Color, depth and stencil buffer selected.
	};

	inline constexpr TargetBufferFlags getTargetBufferFlagsAt(size_t index) noexcept {
		if (index == 0u) return TargetBufferFlags::COLOR0;
		if (index == 1u) return TargetBufferFlags::COLOR1;
		if (index == 2u) return TargetBufferFlags::COLOR2;
		if (index == 3u) return TargetBufferFlags::COLOR3;
		if (index == 4u) return TargetBufferFlags::COLOR4;
		if (index == 5u) return TargetBufferFlags::COLOR5;
		if (index == 6u) return TargetBufferFlags::COLOR6;
		if (index == 7u) return TargetBufferFlags::COLOR7;
		if (index == 8u) return TargetBufferFlags::DEPTH;
		if (index == 9u) return TargetBufferFlags::STENCIL;
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
		Filter  magFilter : 1 = Filter::Linear;
		Filter  minFilter : 1 = Filter::Linear;
		Filter  mipmapMode : 1 = Filter::Linear;
		SamplerAddressMode    addressModeU : 3 = SamplerAddressMode::Repeat;
		SamplerAddressMode    addressModeV : 3 = SamplerAddressMode::Repeat;
		SamplerAddressMode    addressModeW : 3 = SamplerAddressMode::Repeat;
		bool	compareEnable : 1 = false;
		CompareOp	compareOp : 3 = CompareOp::Never;
		BorderColor	borderColor : 3 = BorderColor::FloatTransparentBlack;
		bool    unnormalizedCoordinates : 1 = false;
		bool	anisotropyEnable : 1 = false;
		uint8_t reserve{};

		SamplerInfo() = default;

		SamplerInfo(Filter filter, SamplerAddressMode addressMode) {
			magFilter = filter;
			minFilter = filter;
			mipmapMode = filter;
			addressModeU = addressMode;
			addressModeV = addressMode;
			addressModeW = addressMode;
		}

		bool operator == (const SamplerInfo& other) const {
			return std::memcmp(this, &other, sizeof(SamplerInfo)) == 0;
		}
	};

	static_assert(sizeof(SamplerInfo) == 4);

	enum class ResourceState : uint32_t
    {
        CommonResource          = 0x0,          ///< Common resource state.
        VertexBufferResource    = 0x1 << 0,     ///< Vertex buffer resource state.
        ConstantBufferResource  = 0x1 << 1,     ///< Constant buffer resource state.
        IndexBufferResource     = 0x1 << 2,     ///< Index buffer resource state.
        RenderTargetResource    = 0x1 << 3,     ///< Render target resource state.
        UnorderedAccess         = 0x1 << 4,     ///< Unordered access resource state.
        DepthWrite              = 0x1 << 5,     ///< Depth write resource state.
        DepthRead               = 0x1 << 6,     ///< Depth read resource state.
        NonPixelShaderResource  = 0x1 << 7,     ///< Non-pixel shader resource state.
        PixelShaderResource     = 0x1 << 8,     ///< Pixel shader resource state.
        IndirectArgument        = 0x1 << 9,     ///< Indirect argument resource state.
        CopyDest                = 0x1 << 10,    ///< Copy destination resource state.
        CopySource              = 0x1 << 11,    ///< Copy source resource state.
        ResolveDest             = 0x1 << 12,    ///< Resolve destination resource state.
        ResolveSource           = 0x1 << 13,    ///< Resolve source resource state.
        RTAccelerationStruct    = 0x1 << 14,    ///< Ray tracing acceleration structure resource state.
        ShadingRateSource       = 0x1 << 15,    ///< Shading rate source resource state.
        GenericRead             = 0x1 << 16,    ///< Generic read resource state.
        Present                 = 0x1 << 17,    ///< Present resource state.

        // Special cases
        DepthShaderResource     = DepthRead | NonPixelShaderResource | PixelShaderResource, ///< Depth shader resource state.
        ShaderResource          = NonPixelShaderResource | PixelShaderResource,             ///< Shader resource (general) state
		Undefined = (ResourceState) - 1,
	};
	
	struct DescriptorSetLayoutBinding {
		uint32_t        binding;
		DescriptorType  descriptorType;
		uint32_t        descriptorCount;
		ShaderStage     stageFlags;
		void*			pImmutableSamplers;
		String		name;
	};

	enum class BarrierType
    {
        Transition,
        Aliasing,
        UAV,
    };

	class HwResource;
	
    enum class ResourceType
    {
        Unknown,
        Image,
        Buffer,
    };

    struct Barrier
    {
        BarrierType     type;
        const HwResource*  pResource;
        ResourceState   sourceState;
        ResourceState   destState;
        uint32_t        subResource = 0xffffffff;

        static Barrier Transition(const HwResource* pRes, ResourceState srcState, ResourceState dstState, uint32_t subResource = 0xffffffff)
        {
            Barrier barrier = { BarrierType::Transition, pRes, srcState, dstState, subResource };
            return barrier;
        }

        static Barrier UAV(const HwResource* pRes)
        {
            Barrier barrier = {BarrierType::UAV, pRes, ResourceState::UnorderedAccess, ResourceState::UnorderedAccess};
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

}

template<> struct utils::EnableBitMaskOperators<mygfx::BufferUsage>
: public std::true_type {};

template<> struct utils::EnableBitMaskOperators<mygfx::TextureUsage>
: public std::true_type {};

template<> struct utils::EnableBitMaskOperators<mygfx::VertexAttribute>
: public std::true_type {};

template<> struct utils::EnableBitMaskOperators<mygfx::ColorComponent>
: public std::true_type {};

template<> struct utils::EnableBitMaskOperators<mygfx::ShaderStage>
: public std::true_type {};

template<> struct utils::EnableBitMaskOperators<mygfx::DescriptorType>
: public std::true_type {};

template<> struct utils::EnableBitMaskOperators<mygfx::TargetBufferFlags>
: public std::true_type {};

template<> struct utils::EnableBitMaskOperators<mygfx::ResourceState>
: public std::true_type {};

