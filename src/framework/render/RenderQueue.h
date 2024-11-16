#pragma once

#include "Core/Fwd.h"
#include "RenderDefs.h"
#include "core/Maths.h"
#include <array>

namespace mygfx {

class GraphicsApi;
class HwRenderQueue;
struct Primitive;
class Renderable;

struct RenderList {
    void init();
    void clear();
    void draw(GraphicsApi& cmd, RenderingContext& ctx);
    Vector<Renderable*> renderables;
    Ref<HwRenderQueue> renderQueue;
};

struct RenderQueue {
    void init();
    void clear();
    void addRenderable(Renderable* renderable);
    void draw(GraphicsApi& cmd, RenderingContext& ctx, uint32_t renderableMask = 0xffffffff);

    std::array<RenderList, MAX_RENDER_QUEUE_COUNT> mRenderLists;
};

}