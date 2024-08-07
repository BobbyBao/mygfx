#pragma once
#include <stdint.h>
#include "GraphicsDefs.h"

namespace mygfx {

	struct PrimitiveState {
		PrimitiveTopology primitiveTopology : 4 = PrimitiveTopology::TRIANGLE_LIST;
		bool restartEnable : 1 = false;
		
		auto operator<=>(PrimitiveState const&) const = default;
	};

	//static_assert(sizeof(PrimitiveState) == 1);

	struct RasterState {
		PolygonMode polygonMode : 2 = PolygonMode::FILL;
		CullMode cullMode : 2 = CullMode::BACK;
		FrontFace frontFace : 1 = FrontFace::COUNTER_CLOCKWISE;
		bool depthBiasEnable : 1 = false;

		bool alphaToCoverageEnable : 1 = false;
		bool alphaToOneEnable : 1 = false;
		bool rasterizerDiscardEnable : 1 = false;
		SampleCount rasterizationSamples : 6 = SampleCount::SAMPLE_1;
		
		auto operator<=>(RasterState const&) const = default;
	};

	static_assert(sizeof(RasterState) == 2);
	
	enum class BlendMode : uint8_t
	{
		NONE = 0,
		MASK,
		ADD,
		MULTIPLY,
		ALPHA,
		ADD_ALPHA,
		CONSTANT_COLOR,
		PREMUL_ALPHA,
		INV_DEST_ALPHA,
		SUBTRACT,
		SUBTRACT_ALPHA,
	};

	struct ColorBlendState {

		uint32_t colorBlendEnable : 1 = false;

		BlendFactor srcColorBlendFactor : 5 = BlendFactor::ONE;
		BlendFactor dstColorBlendFactor : 5 = BlendFactor::ONE;
		BlendOp colorBlendOp : 3 = BlendOp::ADD;

		BlendFactor srcAlphaBlendFactor : 5 = BlendFactor::ONE;
		BlendFactor dstAlphaBlendFactor : 5 = BlendFactor::ONE;
		BlendOp alphaBlendOp : 3 = BlendOp::ADD;

		ColorComponent colorWrite : 4 = ColorComponent::RGBA;

		static ColorBlendState get(BlendMode blendMode);

		auto operator<=>(ColorBlendState const&) const = default;
	};

	static_assert(sizeof(ColorBlendState) == 4);

	struct DepthState {
		bool    depthTestEnable : 1 = true;
		bool    depthWriteEnable : 1 = true;
		CompareOp depthCompareOp : 4 = CompareOp::LESS_OR_EQUAL;
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
		HwProgram* program;
		VertexAttribute vertexSemantic = VertexAttribute::ALL;
		PrimitiveState primitiveState {};
		ColorBlendState colorBlendState {};		
		RasterState rasterState {};
		DepthState depthState {};
		StencilState* stencilState {};
	};

}