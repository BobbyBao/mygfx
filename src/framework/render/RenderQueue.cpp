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

void RenderList::draw(GraphicsApi& cmd, uint32_t perView)
{
    auto& renderCmds = renderQueue->getWriteCommands();
    for (auto& renderable : renderables) {
        ObjectUniforms objectUniforms {
            .worldMatrix = renderable->getOwner()->getWorldTransform(),
            .normalMatrix = transpose(inverse(objectUniforms.worldMatrix))
        };

        uint32_t perObject = gfxApi().allocConstant(objectUniforms);

        for (auto& prim : renderable->primitives) {
            if (prim.indirectBuffer != nullptr) {
                drawIndirect(cmd, prim, perView, perObject);
                continue;
            }

            auto& rc = renderCmds.emplace_back();
            rc.renderPrimitive = prim.renderPrimitive;
            rc.pipelineState = prim.material->getPipelineState();
            rc.firstInstance = prim.firstInstance;
            rc.instanceCount = prim.instanceCount;
            rc.uniforms.set(perView, perObject, prim.material->getMaterialUniforms(), prim.primitiveUniforms);
        }
    }

    cmd.drawBatch(renderQueue);
}

void RenderList::drawIndirect(GraphicsApi& cmd, Primitive& primitive, uint32_t perView, uint32_t perObject)
{
    cmd.bindPipelineState(primitive.material->getPipelineState());
    cmd.bindUniforms(Uniforms { perView, perObject, primitive.material->getMaterialUniforms(), primitive.primitiveUniforms });
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

void RenderQueue::draw(GraphicsApi& cmd, uint32_t perView)
{
    for (auto& renderList : mRenderLists) {
        if (renderList.renderables.empty()) {
            continue;
        }

        renderList.draw(cmd, perView);
    }
}

}
