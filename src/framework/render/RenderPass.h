#pragma once
#include "core/Object.h"
#include "RenderDefs.h"

namespace mygfx {
    
class GraphicsApi;

class RenderPass : public Object {
public:
    RenderPass() = default;

    PROPERTY_GET_SET_1(PassName, Name)
    
    template <typename T, typename... Args>
    RenderPass& addPass(Args... args)
    {
        T* pass = new T(std::forward<Args>(args)...);
        addPass(pass);
        return *this;
    }
    
    void addPass(RenderPass* pass);
    void draw(GraphicsApi& cmd, RenderingContext& ctx);

protected:
    virtual void onPreDraw(GraphicsApi& cmd, RenderingContext& ctx) { }
    virtual void onDraw(GraphicsApi& cmd, RenderingContext& ctx) = 0;
    virtual void onPostDraw(GraphicsApi& cmd, RenderingContext& ctx) { }

    PassName mName;
    Vector<Ref<RenderPass>> mChildren;
};

class SceneRenderPass : public RenderPass {
public:
    SceneRenderPass(PassName passName = {});

    PROPERTY_GET_SET(uint32_t, RenderableMask)
protected:    
    void onDraw(GraphicsApi& cmd, RenderingContext& ctx) override;

    uint32_t mRenderableMask = 0xffffffff;
};

}