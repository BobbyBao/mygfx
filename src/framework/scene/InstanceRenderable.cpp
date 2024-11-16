#include "InstanceRenderable.h"
#include "GraphicsApi.h"
#include "Scene.h"
#include "render/RenderDefs.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/Shader.h"
#include "resource/Texture.h"

namespace mygfx {

InstanceRenderable::InstanceRenderable()
{
}

Object* InstanceRenderable::createObject()
{
    return new InstanceRenderable();
}

void InstanceRenderable::cloneProcess(Object* destObj)
{
    MeshRenderable::cloneProcess(destObj);

    InstanceRenderable* dest = (InstanceRenderable*)destObj;
    dest->mMaterial = mMaterial;
    dest->mInstanceData = mInstanceData;
    dest->updateInstanceBuffers();
}

void InstanceRenderable::setInstanceData(Vector<Vector<InstanceData>>&& instanceData)
{
    mInstanceData = std::move(instanceData);

    updateInstanceBuffers();
}

void InstanceRenderable::updateRenderable()
{
    MeshRenderable::updateRenderable();

    for (auto i = 0; i < mInstanceBuffers.size(); i++) {
        auto& instanceBuffer = mInstanceBuffers[i];

        if (i < primitives.size()) {
            auto& primitive = primitives[i];
            primitive.instanceCount = (uint32_t)instanceBuffer->count();
        }
    }
}

void InstanceRenderable::updateInstanceBuffers()
{
    mInstanceBuffers.clear();

    Vector<Matrix4> temp;
    for (auto i = 0; i < mInstanceData.size(); i++) {
        auto& instData = mInstanceData[i];

        temp.clear();

        for (auto& d : instData) {
            mat4 localTransform = glm::scale(glm::mat4_cast(d.rotation), vec3 { d.scale });
            localTransform[3] = { d.position, 1.0f };
            temp.emplace_back(localTransform);
        }

        HwBuffer* instanceBuffer = gfxApi().createBuffer1(BufferUsage::STORAGE | BufferUsage::SHADER_DEVICE_ADDRESS,
            MemoryUsage::GPU_ONLY, Span<Matrix4> { temp });
        mInstanceBuffers.emplace_back(instanceBuffer);

        transformBuffer = instanceBuffer->deviceAddress;

        if (i < primitives.size()) {
            auto& primitive = primitives[i];
            primitive.instanceCount = (uint32_t)instData.size();
        }
    }
}

IndirectRenderable::IndirectRenderable()
{
    mRenderer = this;
}

Object* IndirectRenderable::createObject()
{
    return new IndirectRenderable();
}

void IndirectRenderable::cloneProcess(Object* destObj)
{
    InstanceRenderable::cloneProcess(destObj);

    IndirectRenderable* dest = (IndirectRenderable*)destObj;
    //dest->mMaterial = mMaterial;
}

void IndirectRenderable::draw(GraphicsApi& cmd, RenderingContext& ctx)
{
    ObjectUniforms objectUniforms {
        .worldMatrix = getOwner()->getWorldTransform(),
        .normalMatrix = transpose(inverse(objectUniforms.worldMatrix)),
        .transformBuffer = transformBuffer,
    };
    
    auto& renderPrimitive = mMesh->renderPrimitives[0];

    uint32_t perObject = gfxApi().allocConstant(objectUniforms);
    cmd.bindPipelineState(mMaterial->getPipelineState());
    cmd.bindUniforms(Uniforms { ctx.perView, perObject, mMaterial->getMaterialUniforms() });
    auto indirectBuffer = mIndirectBuffer->getIndirectBuffer();
    cmd.drawIndirectPrimitive(renderPrimitive, indirectBuffer, 0ull, (uint32_t)indirectBuffer->count(), indirectBuffer->stride);
}

}