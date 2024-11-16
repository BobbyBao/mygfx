#include "RenderQueue.h"
#include "Framework.h"
#include "scene/InstanceRenderable.h"

namespace mygfx {

void RenderList::init()
{
    if (renderQueue == nullptr) {
        renderQueue = new HwRenderQueue();
    }
}

void RenderList::clear()
{
    renderables.clear();
    renderQueue->clear();
}

void RenderList::draw(GraphicsApi& cmd, RenderingContext& ctx)
{
    auto& renderCmds = renderQueue->getWriteCommands();
    for (auto& renderable : renderables) {
        auto renderer = renderable->getRenderer();
        if (renderer) {
            renderer->draw(cmd, ctx);
            continue;
        }

        ObjectUniforms objectUniforms {
            .worldMatrix = renderable->getOwner()->getWorldTransform(),
            .normalMatrix = transpose(inverse(objectUniforms.worldMatrix)),
            .transformBuffer =  renderable->transformBuffer,
        };

        uint32_t perObject = gfxApi().allocConstant(objectUniforms);

        for (auto& prim : renderable->primitives) {
            auto& rc = renderCmds.emplace_back();
            rc.renderPrimitive = prim.renderPrimitive;
            rc.pipelineState = prim.material->getPipelineState();
            rc.instanceCount = prim.instanceCount;
            rc.uniforms.set(ctx.perView, perObject, prim.material->getMaterialUniforms(), prim.primitiveUniforms);
        }
    }

    cmd.drawBatch(renderQueue);
}

void RenderQueue::init()
{
    for (auto& renderList : mRenderLists) {
        renderList.init();
    }
}

void RenderQueue::clear()
{
    for (auto& renderList : mRenderLists) {
        renderList.clear();
    }
}

void RenderQueue::addRenderable(Renderable* renderable)
{
    mRenderLists[renderable->getRenderableType()].renderables.push_back(renderable);
}

void RenderQueue::draw(GraphicsApi& cmd, RenderingContext& ctx, uint32_t renderableMask)
{
    for (int i = 0; i < MAX_RENDER_QUEUE_COUNT; i++) {
        if ((renderableMask & (1 << i)) == 0) {
            continue;
        }

        auto& renderList = mRenderLists[i];
        if (!renderList.renderables.empty()) {
            renderList.draw(cmd, ctx);
        }

    }
}

}
