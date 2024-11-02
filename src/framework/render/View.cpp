#include "View.h"
#include "Framework.h"

namespace mygfx {

View::View(uint16_t width, uint16_t height, Format format, TextureUsage usage, SampleCount msaa)
{
    mRenderTexture = Texture::createRenderTexture(width, height, Format::R8G8B8A8_UNORM, TextureUsage::SAMPLED);

    RenderTargetDesc desc {
        .width = width,
        .height = height
    };

    desc.colorAttachments.emplace_back(mRenderTexture->getRTV());
    mRenderTarget = gfxApi().createRenderTarget(desc);
}

View::View(HwSwapchain* swapChain)
{
    mSwapchain = swapChain;
    mRenderTarget = swapChain->renderTarget;
}

void View::setScene(Scene* scene)
{
    mScene = scene;
}

void View::setCamera(Camera* camera)
{
    mCamera = camera;
    mRenderQueue.init();
}

void View::update(double delta)
{
    if (mCamera == nullptr) {
        return;
    }

    mFrameUniforms.viewMatrix = mCamera->getViewMatrix();
    mFrameUniforms.projectionMatrix = mCamera->getProjMatrix();
    mFrameUniforms.viewProjectionMatrix = mFrameUniforms.projectionMatrix * mFrameUniforms.viewMatrix;
    mFrameUniforms.invViewMatrix = inverse(mFrameUniforms.viewMatrix);
    mFrameUniforms.invProjectionMatrix = inverse(mFrameUniforms.projectionMatrix);
    mFrameUniforms.invViewProjectionMatrix = inverse(mFrameUniforms.viewProjectionMatrix);

    mFrameUniforms.camera = mCamera->getOwner()->getWorldPosition();
    mFrameUniforms.nearZ = mCamera->getNearPlane();
    mFrameUniforms.cameraDir = mCamera->getDirection();
    mFrameUniforms.farZ = mCamera->getFarPlane();

    mFrameUniforms.screenSize = { mRenderTarget->width, mRenderTarget->height };
    mFrameUniforms.invScreenSize = { 1.0f / mFrameUniforms.screenSize.x, 1.0f / mFrameUniforms.screenSize.y };

    if (mScene == nullptr) {
        return;
    }

    mRenderQueue.clear();

    auto skybox = mScene->getSkybox();
    if (skybox) {

        auto cubeMap = skybox->getCubeMap();
        if (cubeMap) {
            mFrameUniforms.ggxEnvTexture = cubeMap->index();
            mFrameUniforms.mipCount = cubeMap->textureData().mipMapCount;
        }

        auto irr = skybox->getIrrMap();
        if (irr) {
            mFrameUniforms.lambertianEnvTexture = irr->index();
        }

        auto lut = skybox->getGGXLUT();
        if (lut) {
            mFrameUniforms.ggxLUTTexture = lut->index();
        }
    }

    for (auto& renderable : mScene->getRenderables()) {
        mRenderQueue.addRenderable(renderable);
    }
}

void View::render(GraphicsApi& cmd)
{
    uint32_t perView = gfxApi().allocConstant(mFrameUniforms);

    RenderingContext ctx{
        .perView = perView
    };

    mRenderQueue.draw(cmd, ctx);
}

}