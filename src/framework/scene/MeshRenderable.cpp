#include "MeshRenderable.h"
#include "Node.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/Shader.h"

namespace mygfx {

Object* MeshRenderable::createObject()
{
    return new MeshRenderable();
}

void MeshRenderable::cloneProcess(Object* destObj)
{
    Renderable::cloneProcess(destObj);

    MeshRenderable* dest = static_cast<MeshRenderable*>(destObj);
    dest->setMesh(mMesh);
}

void MeshRenderable::setMesh(Mesh* m)
{
    mMesh = m;

    updateRenderable();
}

void MeshRenderable::updateRenderable()
{
    primitives.clear();

    for (uint32_t i = 0; i < mMesh->getSubMeshCount(); i++) {
        auto& subMesh = mMesh->getSubMeshAt(i);
        auto& primitive = primitives.emplace_back();
        primitive.renderPrimitive = mMesh->renderPrimitives[i];
        primitive.material = subMesh.material;
    }
}

Ref<Node> MeshRenderable::createCube(float size)
{
    Ref<Mesh> mesh(Mesh::createCube(size));
    DefineList macros;
    macros.add("HAS_TEXCOORD_0_VEC2", 1)
        .add("HAS_NORMAL_VEC3", 2)
        .add("MATERIAL_METALLICROUGHNESS")
        .add("USE_IBL");

    auto shader = ShaderEffect::fromFile("shaders/primitive.vert", "shaders/pbr.frag", &macros);
    shader->getMainPass()->setVertexInput({ Format::R32G32B32_SFLOAT, Format::END, Format::R32G32_SFLOAT, Format::END, Format::R32G32B32_SFLOAT });

    auto material = new Material(shader, "MaterialUniforms");
    mesh->setMaterial(material);
    material->setShaderParameter("u_BaseColorFactor", vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
    material->setShaderParameter("u_MetallicFactor", 0.0f);
    material->setShaderParameter("u_RoughnessFactor", 0.5f);

    Ref<Node> node(new Node());
    node->addComponent<MeshRenderable>()->setMesh(mesh);
    return node;
}

}