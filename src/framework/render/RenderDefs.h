#pragma once
#include "core/Maths.h"
#include "PassID.h"

namespace mygfx {

static constexpr uint32_t MAX_RENDER_QUEUE_COUNT = 32;


struct RenderingContext {
    PassID pass = 0;
    uint32_t perView = 0;
};

struct ObjectUniforms {
    mat4 worldMatrix;
    mat3x4 normalMatrix;
    uint64_t transformBuffer = 0;
    uint64_t normalBuffer = 0;
};


}