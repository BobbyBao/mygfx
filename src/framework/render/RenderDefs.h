#pragma once
#include "core/Maths.h"
#include "PassName.h"

namespace mygfx {

static constexpr uint32_t MAX_RENDER_QUEUE_COUNT = 32;


struct RenderingContext {
    PassName pass;
    uint32_t perView = 0;
    struct RenderQueue& renderQueue;
};

struct ObjectUniforms {
    mat4 worldMatrix;
    mat3x4 normalMatrix;
    uint64_t transformBuffer = 0;
    uint64_t skinningBuffer = 0;
};


}