#include "Renderable.h"
#include "GraphicsApi.h"
#include "Scene.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/Shader.h"

namespace mygfx {

Renderable::Renderable() = default;

Object* Renderable::createObject()
{
    return new Renderable();
}

void Renderable::cloneProcess(Object* destNode)
{
    Renderable* renderable = static_cast<Renderable*>(destNode);
    renderable->setMesh(mMesh);
}

void Renderable::setMesh(Mesh* m)
{
    mMesh = m;
    primitives.clear();

    for (uint32_t i = 0; i < mMesh->getSubMeshCount(); i++) {
        auto& subMesh = mMesh->getSubMeshAt(i);
        auto& primitive = primitives.emplace_back();
        primitive.renderPrimitive = mMesh->renderPrimitives[i];
        primitive.material = subMesh.material;
    }
}

void Renderable::onAddToScene(Scene* scene)
{
    scene->mRenderables.insert(this);
}

void Renderable::onRemoveFromScene(Scene* scene)
{
    scene->mRenderables.erase(this);
}

Ref<Node> Renderable::createCube(float size)
{
    Ref<Mesh> mesh(Mesh::createCube(size));
    DefineList macros;
    macros.add("HAS_TEXCOORD_0_VEC2", 1)
        .add("HAS_NORMAL_VEC3", 2)
        .add("MATERIAL_METALLICROUGHNESS")
        .add("LINEAR_OUTPUT")
        .add("USE_IBL")
        .add("DEBUG_NONE", 0)
        .add("DEBUG", 0);

    auto shader = Shader::fromFile("shaders/primitive.vert", "shaders/pbr.frag", &macros);
    shader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::END, Format::R32G32_SFLOAT, Format::END, Format::R32G32B32_SFLOAT });

    auto material = new Material(shader, "MaterialUniforms");
    mesh->setMaterial(material);
    material->setShaderParameter("u_BaseColorFactor", vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
    material->setShaderParameter("u_MetallicFactor", 0.0f);
    material->setShaderParameter("u_RoughnessFactor", 0.5f);

    Ref<Node> node(new Node());
    node->addComponent<Renderable>()->setMesh(mesh);
    return node;
}

}