#pragma once
#include "core/Object.h"
#include "RenderDefs.h"

namespace mygfx {
    
class GraphicsApi;
class RenderPass;

class RenderGraph : public Object {
public:
    RenderGraph() = default;
    
    template <typename T, typename... Args>
    RenderGraph& addPass(Args... args)
    {
        T* pass = new T(std::forward<Args>(args)...);
        addPass(pass);
        return *this;
    }

    void addPass(RenderPass* pass);

    void draw(GraphicsApi& cmd, RenderingContext& ctx);

    static Ref<RenderGraph> createDefault();
protected:
    Vector<Ref<RenderPass>> mRenderPasses;
};

}