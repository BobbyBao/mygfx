#include "Skybox.h"
#include "Scene.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/Shader.h"
#include "resource/Texture.h"

namespace mygfx {

Skybox::Skybox()
{
}

Object* Skybox::createObject()
{
    return new Skybox();
}

void Skybox::cloneProcess(Object* destObj)
{
    MeshRenderable::cloneProcess(destObj);

    Skybox* skybox = (Skybox*)destObj;
    skybox->mCubeMap = mCubeMap;
    skybox->mIrrMap = mIrrMap;
    skybox->mGGXLUT = mGGXLUT;
}

void Skybox::setCubeMap(Texture* tex)
{
    mCubeMap = tex;
}

void Skybox::setIrrMap(Texture* tex)
{
    mIrrMap = tex;
}

void Skybox::onAddToScene(Scene* scene)
{
    MeshRenderable::onAddToScene(scene);

    if (mMesh == nullptr) {
        auto mesh = Mesh::createFullScreen();

        if (msDefaultShader == nullptr) {
            msDefaultShader = ShaderEffect::load("shaders/Skybox.shader");
        }

        Ref<Material> material = makeRef<Material>(msDefaultShader.get());
        material->setShaderParameter("u_MipLevel", 0);
        material->setShaderParameter("u_EnvBlurNormalized", 1.0f);
        mesh->setMaterial(material);
        setMesh(mesh);
    }

    if (mGGXLUT == nullptr) {
        mGGXLUT = Texture::createFromFile("textures/lut_ggx.png", SamplerInfo::create(Filter::LINEAR, SamplerAddressMode::CLAMP_TO_EDGE));
    }

    scene->mSkybox = this;
}

void Skybox::onRemoveFromScene(Scene* scene)
{
    MeshRenderable::onRemoveFromScene(scene);

    if (scene->mSkybox == this) {
        scene->mSkybox = nullptr;
    }
}
}