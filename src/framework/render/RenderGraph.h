#pragma once
#include "core/Object.h"
#include "RenderDefs.h"

namespace mygfx {
    
class GraphicsApi;
class RenderPass;

class RenderGraph : public Object {
public:
    RenderGraph() = default;

    void draw(GraphicsApi& cmd, RenderingContext& ctx);

    static Ref<RenderGraph> createDefault();
protected:
    Vector<Ref<RenderPass>> mRenderPasses;
};

}