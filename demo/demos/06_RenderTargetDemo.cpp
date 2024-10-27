#include "DemoApp.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"

namespace mygfx::demo {

class RenderTargetDemo : public Demo {
public:
    Ref<Mesh> mMesh;
    Ref<Shader> mShader;
    Ref<Texture> mRenderTexture;
    Ref<HwRenderTarget> mRenderTarget;

    void start() override
    {
        mShader = ShaderLibs::getSimpleLightShader();
        mMesh = Mesh::createCube();
        mRenderTexture = Texture::createRenderTexture(1024, 1024, Format::R8G8B8A8_UNORM, TextureUsage::SAMPLED);

        RenderTargetDesc desc {
            .width = 1024,
            .height = 1024
        };

        desc.colorAttachments.emplace_back(mRenderTexture->getRTV());
        mRenderTarget = gfxApi().createRenderTarget(desc);
    }

    void gui() override
    {
        if (ImGui::Begin("RenderTarget")) {
            ImGui::Texture(mRenderTexture, { 1024, 1024 });
        }
        ImGui::End();
    }

    void preDraw(GraphicsApi& cmd) override
    {
        auto w = mApp->getWidth();
        auto h = mApp->getHeight();
        float aspect = w / (float)h;
        auto vp = glm::ortho(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);

        uint32_t perView = gfxApi().allocConstant(vp);

        auto world = identity<mat4>();

        uint32_t perObject = gfxApi().allocConstant(world);
        uint32_t perMaterial = gfxApi().allocConstant(Texture::Red->index());

        RenderPassInfo renderInfo {
            .clearFlags = TargetBufferFlags::ALL,
            .clearColor = { 0.25f, 0.25f, 0.25f, 1.0f }
        };

        renderInfo.viewport = { .left = 0, .top = 0, .width = 1024, .height = 1024 };

        cmd.beginRendering(mRenderTarget, renderInfo);

        cmd.bindPipelineState(mShader->pipelineState);
        cmd.bindUniforms({ perView, perObject, perMaterial });

        for (auto& prim : mMesh->renderPrimitives) {
            cmd.drawPrimitive(prim);
        }

        cmd.endRendering(mRenderTarget);
    }
};

DEF_DEMO(RenderTargetDemo, "RenderTarget Demo");
}