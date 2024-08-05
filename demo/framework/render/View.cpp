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

	void View::setEnvMap(Texture* cubeMap) {

	}

	void View::update(double delta) {

		mFrameUniforms.viewMatrix = mCamera->getViewMatrix();
		mFrameUniforms.projectionMatrix = mCamera->getProjMatrix();
		mFrameUniforms.viewProjectionMatrix = mFrameUniforms.projectionMatrix * mFrameUniforms.viewMatrix;
		mFrameUniforms.invViewMatrix = inverse(mFrameUniforms.viewMatrix);
		mFrameUniforms.invProjectionMatrix = inverse(mFrameUniforms.projectionMatrix);
		mFrameUniforms.invViewProjectionMatrix = inverse(mFrameUniforms.viewProjectionMatrix);

		mFrameUniforms.camera = mCamera->getWorldPosition();
		mFrameUniforms.nearZ = mCamera->getNearPlane();
		mFrameUniforms.cameraDir = mCamera->getDirection();
		mFrameUniforms.farZ = mCamera->getFarPlane();

		mFrameUniforms.screenSize = { mRenderTarget->width, mRenderTarget->height };
		mFrameUniforms.invScreenSize = { 1.0f / mFrameUniforms.screenSize.x, 1.0f / mFrameUniforms.screenSize.y };
	}

	void View::render(GraphicsApi& cmd) {

		uint32_t perView = gfxApi().allocConstant(mFrameUniforms);

		for (auto renderable : mScene->renderables) {

			auto& world = renderable->getWorldTransform();
			uint32_t perDraw = gfxApi().allocConstant(world);

			for (auto& prim : renderable->primitives) {

				uint32_t perMaterial = prim.material->getMaterialUniforms();

				cmd.bindPipelineState(prim.material->getPipelineState());			
				cmd.bindUniforms(perMaterial == 0 ? Uniforms{ perView, perDraw } : Uniforms{ perView, perDraw, perMaterial });
				cmd.drawPrimitive(prim.renderPrimitive);
			}
		}

	}
}