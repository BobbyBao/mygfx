#include "PipelineState.h"


namespace mygfx {

	
	constexpr static bool blendEnable[] =
	{
		false,
		false,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true
	};

	constexpr static BlendFactor sColorSrcBlend[] =
	{
		BlendFactor::One,
		BlendFactor::One,
		BlendFactor::One,		//Add
		BlendFactor::DstColor,
		BlendFactor::SrcAlpha,
		BlendFactor::SrcAlpha,	//AddAlpha
		BlendFactor::One,		//AddConstantColor
		BlendFactor::One,
		BlendFactor::OneMinusDstAlpha,
		BlendFactor::One,
		BlendFactor::SrcAlpha,
	};

	constexpr static BlendFactor sColorDestBlend[] =
	{
		BlendFactor::Zero,
		BlendFactor::Zero,
		BlendFactor::One,				//Add
		BlendFactor::Zero,
		BlendFactor::OneMinusSrcAlpha,
		BlendFactor::One,				//AddAlpha
		BlendFactor::ConstantColor,		//AddConstantColor
		BlendFactor::OneMinusSrcAlpha,
		BlendFactor::DstAlpha,
		BlendFactor::One,
		BlendFactor::One,
	};

	constexpr static BlendOp sColorBlendOp[] =
	{
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::Add,			//Add
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::Add,			//AddAlpha
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::ReverseSubtract,
		BlendOp::ReverseSubtract,
	};

	constexpr static BlendFactor sAlphaSrcBlend[] =
	{
		BlendFactor::One,
		BlendFactor::One,
		BlendFactor::One,			//Add

		BlendFactor::DstColor,
		BlendFactor::SrcAlpha,
		BlendFactor::SrcAlpha,		//AddAlpha
		BlendFactor::One,
		BlendFactor::One,
		BlendFactor::OneMinusDstAlpha,
		BlendFactor::One,
		BlendFactor::SrcAlpha,
	};

	constexpr static BlendFactor sAlphaDestBlend[] =
	{
		BlendFactor::Zero,
		BlendFactor::Zero,
		BlendFactor::Zero,				//Add
		BlendFactor::Zero,
		BlendFactor::OneMinusSrcAlpha,
		BlendFactor::One,				//AddAlpha
		BlendFactor::One,
		BlendFactor::OneMinusSrcAlpha,
		BlendFactor::DstAlpha,
		BlendFactor::One,
		BlendFactor::One,
	};

	constexpr static BlendOp sAlphaBlendOp[] =
	{
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::Add,	//Add
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::Add,	//AddAlpha
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::Add,
		BlendOp::ReverseSubtract,
		BlendOp::ReverseSubtract,
	};

	
	ColorBlendState ColorBlendState::get(BlendMode blendMode) {
		ColorBlendState blendState {
			.colorBlendEnable =	blendEnable[(int)blendMode],
			.srcColorBlendFactor =	sColorSrcBlend[(int)blendMode],
			.dstColorBlendFactor =	sColorDestBlend[(int)blendMode],		
			.colorBlendOp =	sColorBlendOp[(int)blendMode],
			.srcAlphaBlendFactor =	sAlphaSrcBlend[(int)blendMode],
			.dstAlphaBlendFactor =	sAlphaDestBlend[(int)blendMode],	
			.alphaBlendOp =	sAlphaBlendOp[(int)blendMode],	
			.colorWrite =	ColorComponent::RGBA
		};
		return blendState;
	}
}