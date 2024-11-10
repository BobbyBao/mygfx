#pragma once
#include "core/Object.h"
#include "RenderDefs.h"

namespace mygfx {
    
class GraphicsApi;

class RenderPass : public Object {
public:
    RenderPass() = default;

    PROPERTY_GET_SET_1(PassName, Name)
    
    void draw(GraphicsApi& cmd, RenderingContext& ctx);

protected:
    
    virtual void onDraw(GraphicsApi& cmd, RenderingContext& ctx) = 0;

    PassName mName;

    Vector<Ref<RenderPass>> mChildren;
};

class SceneRenderPass : public RenderPass {
public:

    PROPERTY_GET_SET(uint32_t, RenderableMask)
protected:    
    void onDraw(GraphicsApi& cmd, RenderingContext& ctx) override;
    uint32_t mRenderableMask = 0xffffffff;
};

}