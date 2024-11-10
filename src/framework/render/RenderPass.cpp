#include "RenderPass.h"
#include "RenderQueue.h"

namespace mygfx {

void RenderPass::draw(GraphicsApi& cmd, RenderingContext& ctx)
{
    for (auto& pass : mChildren) {
        pass->draw(cmd, ctx);
    }

    onDraw(cmd, ctx);
}

void SceneRenderPass::onDraw(GraphicsApi& cmd, RenderingContext& ctx) {
    
    ctx.renderQueue.draw(cmd, ctx);
}

}