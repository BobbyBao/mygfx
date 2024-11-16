#include "RenderPass.h"
#include "RenderQueue.h"

namespace mygfx {

void RenderPass::addPass(RenderPass* pass)
{
    mChildren.emplace_back(pass);
}

void RenderPass::draw(GraphicsApi& cmd, RenderingContext& ctx)
{
    for (auto& pass : mChildren) {
        pass->draw(cmd, ctx);
    }
    
    onPreDraw(cmd, ctx);
    onDraw(cmd, ctx);
    onPostDraw(cmd, ctx);
}

SceneRenderPass::SceneRenderPass(PassName passName)
{
    setName(passName);
}

void SceneRenderPass::onDraw(GraphicsApi& cmd, RenderingContext& ctx)
{
    ctx.pass = mName;
    ctx.renderQueue.draw(cmd, ctx, mRenderableMask);
}

}