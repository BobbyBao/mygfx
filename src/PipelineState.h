#pragma once
#include <stdint.h>
#include "GraphicsDefs.h"

namespace mygfx {

	struct PrimitiveState {
		PrimitiveTopology primitiveTopology : 4 = PrimitiveTopology::TriangleList;
		bool restartEnable : 1 = false;
		
		auto operator<=>(PrimitiveState const&) const = default;
	};

	//static_assert(sizeof(PrimitiveState) == 1);

	struct RasterState {
		PolygonMode polygonMode : 2 = PolygonMode::Fill;
		CullMode cullMode : 2 = CullMode::Front;
		FrontFace frontFace : 1 = FrontFace::Clockwise;
		bool depthBiasEnable : 1 = false;

		bool alphaToCoverageEnable : 1 = false;
		bool alphaToOneEnable : 1 = false;
		bool rasterizerDiscardEnable : 1 = false;
		SampleCount rasterizationSamples : 6 = SampleCount::_1;
		
		auto operator<=>(RasterState const&) const = default;
	};

	static_assert(sizeof(RasterState) == 2);
	
	enum class BlendMode : uint8_t
	{
		None = 0,
		Mask,
		Add,
		Multiply,
		Alpha,
		AddAlpha,
		ConstantColor,
		PremulAlpha,
		InvdestAlpha,
		Subtract,
		SubtractAlpha,
	};

	struct ColorBlendState {

		uint32_t colorBlendEnable : 1 = false;

		BlendFactor srcColorBlendFactor : 5 = BlendFactor::One;
		BlendFactor dstColorBlendFactor : 5 = BlendFactor::One;
		BlendOp colorBlendOp : 3 = BlendOp::Add;

		BlendFactor srcAlphaBlendFactor : 5 = BlendFactor::One;
		BlendFactor dstAlphaBlendFactor : 5 = BlendFactor::One;
		BlendOp alphaBlendOp : 3 = BlendOp::Add;

		ColorComponent colorWrite : 4 = ColorComponent::RGBA;

		static ColorBlendState get(BlendMode blendMode);

		auto operator<=>(ColorBlendState const&) const = default;
	};

	static_assert(sizeof(ColorBlendState) == 4);

	struct DepthState {
		bool    depthTestEnable : 1 = true;
		bool    depthWriteEnable : 1 = true;
		CompareOp depthCompareOp : 4 = CompareOp::LessOrEqual;
		bool depthBoundsTestEnable : 1 = false;

		auto operator<=>(DepthState const&) const = default;
	};

	struct StencilOpState {
		StencilOp    failOp : 3;
		StencilOp    passOp : 3;
		StencilOp    depthFailOp : 3;
		CompareOp    compareOp : 4;
		uint8_t       compareMask;
		uint8_t       writeMask;
		uint8_t       reference;

		auto operator<=>(StencilOpState const&) const = default;
	};

	struct StencilState {
		bool stencilTestEnable : 1 = false;
		StencilOpState            front;
		StencilOpState            back;

		auto operator<=>(StencilState const&) const = default;
	};

	class HwVertexInput;
	class HwProgram;

	struct PipelineState {
		//HwVertexInput* vertexInput;
		HwProgram* program;
		VertexAttribute vertexSemantic = VertexAttribute::All;
		PrimitiveState primitiveState {};
		ColorBlendState colorBlendState {};		
		RasterState rasterState {};
		DepthState depthState {};
		StencilState* stencilState {};
	};

}