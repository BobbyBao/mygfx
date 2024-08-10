#include "PipelineState.h"

namespace mygfx {

constexpr static bool blendEnable[] = {
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

constexpr static BlendFactor sColorSrcBlend[] = {
    BlendFactor::ONE,
    BlendFactor::ONE,
    BlendFactor::ONE, // Add
    BlendFactor::DST_COLOR,
    BlendFactor::SRC_ALPHA,
    BlendFactor::SRC_ALPHA, // AddAlpha
    BlendFactor::ONE, // AddConstantColor
    BlendFactor::ONE,
    BlendFactor::ONE_MINUS_DST_ALPHA,
    BlendFactor::ONE,
    BlendFactor::SRC_ALPHA,
};

constexpr static BlendFactor sColorDestBlend[] = {
    BlendFactor::ZERO,
    BlendFactor::ZERO,
    BlendFactor::ONE, // Add
    BlendFactor::ZERO,
    BlendFactor::ONE_MINUS_SRC_ALPHA,
    BlendFactor::ONE, // AddAlpha
    BlendFactor::CONSTANT_COLOR, // AddConstantColor
    BlendFactor::ONE_MINUS_SRC_ALPHA,
    BlendFactor::DST_ALPHA,
    BlendFactor::ONE,
    BlendFactor::ONE,
};

constexpr static BlendOp sColorBlendOp[] = {
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::ADD, // Add
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::ADD, // AddAlpha
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::REVERSE_SUBTRACT,
    BlendOp::REVERSE_SUBTRACT,
};

constexpr static BlendFactor sAlphaSrcBlend[] = {
    BlendFactor::ONE,
    BlendFactor::ONE,
    BlendFactor::ONE, // Add

    BlendFactor::DST_COLOR,
    BlendFactor::SRC_ALPHA,
    BlendFactor::SRC_ALPHA, // AddAlpha
    BlendFactor::ONE,
    BlendFactor::ONE,
    BlendFactor::ONE_MINUS_DST_ALPHA,
    BlendFactor::ONE,
    BlendFactor::SRC_ALPHA,
};

constexpr static BlendFactor sAlphaDestBlend[] = {
    BlendFactor::ZERO,
    BlendFactor::ZERO,
    BlendFactor::ZERO, // Add
    BlendFactor::ZERO,
    BlendFactor::ONE_MINUS_SRC_ALPHA,
    BlendFactor::ONE, // AddAlpha
    BlendFactor::ONE,
    BlendFactor::ONE_MINUS_SRC_ALPHA,
    BlendFactor::DST_ALPHA,
    BlendFactor::ONE,
    BlendFactor::ONE,
};

constexpr static BlendOp sAlphaBlendOp[] = {
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::ADD, // Add
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::ADD, // AddAlpha
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::ADD,
    BlendOp::REVERSE_SUBTRACT,
    BlendOp::REVERSE_SUBTRACT,
};

ColorBlendState ColorBlendState::get(BlendMode blendMode)
{
    ColorBlendState blendState {
        .colorBlendEnable = blendEnable[(int)blendMode],
        .srcColorBlendFactor = sColorSrcBlend[(int)blendMode],
        .dstColorBlendFactor = sColorDestBlend[(int)blendMode],
        .colorBlendOp = sColorBlendOp[(int)blendMode],
        .srcAlphaBlendFactor = sAlphaSrcBlend[(int)blendMode],
        .dstAlphaBlendFactor = sAlphaDestBlend[(int)blendMode],
        .alphaBlendOp = sAlphaBlendOp[(int)blendMode],
        .colorWrite = ColorComponent::RGBA
    };
    return blendState;
}
}