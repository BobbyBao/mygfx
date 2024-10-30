#pragma once
#include "core/Maths.h"

namespace mygfx {

static constexpr uint32_t MAX_RENDER_QUEUE_COUNT = 32;

struct RenderingContext {
    uint32_t pass = 0;
    uint32_t perView = 0;
};

struct ObjectUniforms {
    mat4 worldMatrix;
    mat4 normalMatrix;
};

}