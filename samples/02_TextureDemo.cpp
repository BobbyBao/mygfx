#include "DemoApp.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"

namespace mygfx::samples {

class TextureDemo : public Demo {
public:
    Ref<Mesh> mMesh;
    Ref<Shader> mShader;

    void start() override
    {
        mShader = ShaderLibs::getSimpleLightShader();
        mMesh = Mesh::createCube();
    }

    void draw(GraphicsApi& cmd) override
    {
        float w = (float)mApp->getWidth();
        float h = (float)mApp->getHeight();
        float aspect = w / h;
        auto vp = glm::ortho(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);

        uint32_t perView = gfxApi().allocConstant(vp);

        auto world = math::identity<mat4>();

        uint32_t perObject = gfxApi().allocConstant(world);
        uint32_t perMaterial = gfxApi().allocConstant(Texture::Red->index());

        cmd.bindPipelineState(mShader->pipelineState);
        cmd.bindUniforms({ perView, perObject, perMaterial });

        for (auto& prim : mMesh->renderPrimitives) {
            cmd.drawPrimitive(prim, 1, 0);
        }
    }
};

DEF_DEMO(TextureDemo, "Texture Demo");
}