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
            if (prim.indirectBuffer != nullptr) {
                drawIndirect(cmd, prim, ctx, perObject);
                continue;
            }

            auto& rc = renderCmds.emplace_back();
            rc.renderPrimitive = prim.renderPrimitive;
            rc.pipelineState = prim.material->getPipelineState();
            rc.instanceCount = prim.instanceCount;
            rc.uniforms.set(ctx.perView, perObject, prim.material->getMaterialUniforms(), prim.primitiveUniforms);
        }
    }

    cmd.drawBatch(renderQueue);
}

void RenderList::drawIndirect(GraphicsApi& cmd, Primitive& primitive, RenderingContext& ctx, uint32_t perObject)
{
    cmd.bindPipelineState(primitive.material->getPipelineState());
    cmd.bindUniforms(Uniforms { ctx.perView, perObject, primitive.material->getMaterialUniforms(), primitive.primitiveUniforms });
    auto indirectBuffer = primitive.indirectBuffer->getIndirectBuffer();
    cmd.drawIndirectPrimitive(primitive.renderPrimitive, indirectBuffer, 0ull, (uint32_t)indirectBuffer->count(), indirectBuffer->stride);
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
    mRenderLists[renderable->getRenderQueue()].renderables.push_back(renderable);
}

void RenderQueue::draw(GraphicsApi& cmd, RenderingContext& ctx)
{
    for (auto& renderList : mRenderLists) {
        if (renderList.renderables.empty()) {
            continue;
        }

        renderList.draw(cmd, ctx);
    }
}

}
