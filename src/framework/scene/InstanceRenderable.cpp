#include "InstanceRenderable.h"
#include "GraphicsApi.h"
#include "Scene.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/Shader.h"
#include "resource/Texture.h"
#include "render/RenderDefs.h"

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

    updateRenderable();
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
            //primitive.primitiveUniforms = gfxApi().allocConstant<PrimitiveUniforms>({.instanceBuffer = instanceBuffer->deviceAddress});
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
            mat4 localTransform = glm::scale(glm::mat4_cast(d.rotation), vec3{d.scale});
            localTransform[3] = { d.position, 1.0f };
            temp.emplace_back(localTransform);
        }

        HwBuffer* instanceBuffer = gfxApi().createBuffer1(BufferUsage::STORAGE | BufferUsage::SHADER_DEVICE_ADDRESS,
            MemoryUsage::GPU_ONLY, Span { temp });
        mInstanceBuffers.emplace_back(instanceBuffer);

        if (i < primitives.size()) {
            auto& primitive = primitives[i];
            primitive.instanceCount = (uint32_t)instData.size();
            //primitive.primitiveUniforms = gfxApi().allocConstant<PrimitiveUniforms>({.instanceBuffer = instanceBuffer->deviceAddress});
        }
    }


}

}