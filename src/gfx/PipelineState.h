#pragma once
#include "GraphicsDefs.h"
#include <stdint.h>

namespace mygfx {

struct PrimitiveState {
    PrimitiveTopology primitiveTopology : 4 = PrimitiveTopology::TRIANGLE_LIST;
    bool restartEnable : 1 = false;
    uint8_t reserve : 3 = 0;

    auto operator<=>(PrimitiveState const&) const = default;
};

static_assert(sizeof(PrimitiveState) == 1);

struct RasterState {
    PolygonMode polygonMode : 2 = PolygonMode::FILL;
    CullMode cullMode : 2 = CullMode::BACK;
    FrontFace frontFace : 1 = FrontFace::COUNTER_CLOCKWISE;
    bool depthBiasEnable : 1 = false;

    bool alphaToCoverageEnable : 1 = false;
    bool alphaToOneEnable : 1 = false;
    bool rasterizerDiscardEnable : 1 = false;
    uint8_t reserve : 1 = 0;
    SampleCount rasterizationSamples : 6 = SampleCount::SAMPLE_1;

    auto operator<=>(RasterState const&) const = default;
};

static_assert(sizeof(RasterState) == 2);

enum class BlendMode : uint8_t {
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
    BlendOp colorBlendOp : 3 = BlendOp::ADD;
    BlendOp alphaBlendOp : 3 = BlendOp::ADD;
    BlendFactor srcColorBlendFactor : 5 = BlendFactor::ONE;
    BlendFactor dstColorBlendFactor : 5 = BlendFactor::ONE;
    BlendFactor srcAlphaBlendFactor : 5 = BlendFactor::ONE;
    BlendFactor dstAlphaBlendFactor : 5 = BlendFactor::ONE;
    ColorComponent colorWrite : 4 = ColorComponent::RGBA;
    uint16_t colorBlendEnable : 1 = false;
    uint16_t reserve : 1 = 0;

    static ColorBlendState get(BlendMode blendMode);

    auto operator<=>(ColorBlendState const&) const = default;
};

static_assert(sizeof(ColorBlendState) == 4);

struct DepthState {
    bool depthTestEnable : 1 = true;
    bool depthWriteEnable : 1 = true;
    CompareOp depthCompareOp : 4 = CompareOp::LESS_OR_EQUAL;
    bool depthBoundsTestEnable : 1 = false;
    uint8_t reserve : 1 = 0;

    auto operator<=>(DepthState const&) const = default;
};

struct StencilOpState {
    StencilOp failOp : 3;
    StencilOp passOp : 3;
    StencilOp depthFailOp : 3;
    CompareOp compareOp : 4;
    uint8_t compareMask;
    uint8_t writeMask;
    uint8_t reference;

    auto operator<=>(StencilOpState const&) const = default;
};

struct StencilState {
    bool stencilTestEnable : 1 = false;
    StencilOpState front;
    StencilOpState back;

    auto operator<=>(StencilState const&) const = default;
};

enum BlendOverlap : uint16_t {
    UNCORRELATED = 0,
    DISJOINT = 1,
    CONJOINT = 2,
};

struct ColorBlendAdvanced {
    BlendOp advancedBlendOp : 3;
    uint16_t srcPremultiplied : 1 = 0;
    uint16_t dstPremultiplied: 1 = 0;
    uint16_t clampResults : 1 = 0;
    BlendOverlap blendOverlap : 3;
    uint16_t reserve : 1 = 0;
};

class AdvancedState : public HwObject {
public:
    StencilState stencilState {};
    uint8_t colorBlendCount = 0;
    ColorBlendAdvanced colorBlendState[8];
};

class HwVertexInput;
class HwProgram;

struct PipelineState {
    HwProgram* program { nullptr };
    union {
        struct {
            PrimitiveState primitiveState {};
            DepthState depthState {};
            RasterState rasterState {};
            ColorBlendState colorBlendState {};
        };
        uint64_t dynamicStateHash;
    };
    AdvancedState* advanceState { nullptr };
};

}