#include "View.h"
#include "Framework.h"

namespace mygfx {

View::View()
{
    mRenderQueue = new RenderQueue();
}

View::View(HwSwapchain* swapChain)
{
    mSwapchain = swapChain;
    mRenderTarget = swapChain->renderTarget;
    mRenderQueue = new RenderQueue();
}

void View::setScene(Scene* scene)
{
    mScene = scene;
}

void View::setCamera(Camera* camera)
{
    mCamera = camera;
}

void View::update(double delta)
{
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

    mRenderQueue->clear();

    if (mScene == nullptr) {
        return;
    }

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
}

void View::render(GraphicsApi& cmd)
{
    uint32_t perView = gfxApi().allocConstant(mFrameUniforms);

    auto& renderCmds = mRenderQueue->getWriteCommands();
    for (auto& renderable : mScene->getRenderables()) {
        ObjectUniforms objectUniforms {
            .worldMatrix = renderable->getOwner()->getWorldTransform(),
            .normalMatrix = transpose(inverse(objectUniforms.worldMatrix))
        };

        uint32_t perObject = gfxApi().allocConstant(objectUniforms);

        for (auto& prim : renderable->primitives) {
            auto& rc = renderCmds.emplace_back();
            rc.renderPrimitive = prim.renderPrimitive;
            rc.pipelineState = prim.material->getPipelineState();
            rc.uniforms.set(perView, perObject, prim.material->getMaterialUniforms());
        }
    }

    cmd.drawBatch(mRenderQueue);
}

}