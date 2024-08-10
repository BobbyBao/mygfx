#include "View.h"
#include "Framework.h"

namespace mygfx {
	
	View::View() {
	}

	View::View(HwSwapchain* swapChain) {
		mSwapchain = swapChain;
		mRenderTarget = swapChain->renderTarget;
	}

	void View::setScene(Scene* scene) {
		mScene = scene;
	}

	void View::setCamera(Camera* camera) {
		mCamera = camera;
	}

	void View::update(double delta) {

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

		if (mScene->skybox) {
			
			auto cubeMap = mScene->skybox->getCubeMap();
			if (cubeMap) {
				mFrameUniforms.ggxEnvTexture = cubeMap->index();
				mFrameUniforms.mipCount = cubeMap->textureData().mipMapCount;
			}

			auto irr = mScene->skybox->getIrrMap();
			if (irr) {
				mFrameUniforms.lambertianEnvTexture = irr->index();
			}

			auto lut = mScene->skybox->getGGXLUT();
			if (lut) {
				mFrameUniforms.ggxLUTTexture = lut->index();
			}
		}
	}

	void View::render(GraphicsApi& cmd) {

		uint32_t perView = gfxApi().allocConstant(mFrameUniforms);

		for (auto renderable : mScene->renderables) {
			ObjectUniforms objectUniforms;
			objectUniforms.worldMatrix = renderable->getOwner()->getWorldTransform();
			objectUniforms.normalMatrix = transpose(inverse(objectUniforms.worldMatrix));

			uint32_t perObject = gfxApi().allocConstant(objectUniforms);

			for (auto& prim : renderable->primitives) {

				uint32_t perMaterial = prim.material->getMaterialUniforms();

				cmd.bindPipelineState(prim.material->getPipelineState());			
				cmd.bindUniforms(perMaterial == 0xffffffff ? Uniforms{ perView, perObject } : Uniforms{ perView, perObject, perMaterial });
				cmd.drawPrimitive(prim.renderPrimitive);
			}
		}

	}
}