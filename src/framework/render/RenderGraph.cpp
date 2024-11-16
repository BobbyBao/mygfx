#include "RenderGraph.h"
#include "RenderPass.h"

namespace mygfx {

void RenderGraph::addPass(RenderPass* pass)
{
    mRenderPasses.emplace_back(pass);
}

void RenderGraph::draw(GraphicsApi& cmd, RenderingContext& ctx)
{
    for (auto& pass : mRenderPasses) {
        pass->draw(cmd, ctx);
    }
}

Ref<RenderGraph> RenderGraph::createDefault()
{
    Ref<RenderGraph> rg(new RenderGraph());
    rg->mRenderPasses.emplace_back(new SceneRenderPass());
    return rg;
}
}