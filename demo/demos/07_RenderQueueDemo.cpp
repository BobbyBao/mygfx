#include "DemoApp.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"

namespace mygfx::demo {

	class RenderQueueDemo : public Demo {
	public:
		Ref<Mesh> mMesh;
		Ref<Shader> mShader;

		struct Renderable {
			mat4 worldTransform = identity<mat4>();
			int texIndex = 0;
		};

		Vector<Renderable> mRenderables;
		Ref<RenderQueue> mRenderQueue = nullptr;
		Vector<Ref<Texture>> mTextures;

		Result<void> start() override {

			mShader = ShaderLibs::getSimpleLightShader();

			mMesh = Mesh::createCube(1.0f);

			mTextures = Texture::createRandomColorTextures(10);

			int GRID_SIZE_X = 100;
			int GRID_SIZE_Y = 100;
			int GRID_SIZE_Z = 1;

			float SPACE = 2.0f;

			for (int i = 0; i < GRID_SIZE_X; i++) {
				for (int j = 0; j < GRID_SIZE_Y; j++) {
					for (int k = 0; k < GRID_SIZE_Z; k++) {
						auto& renderable = mRenderables.emplace_back();
						renderable.worldTransform = glm::translate(identity<mat4>(),
							{ (i - GRID_SIZE_X / 2) * SPACE, (j - GRID_SIZE_Y / 2) * SPACE, (k - GRID_SIZE_Z / 2) * SPACE });

						renderable.texIndex = mTextures[glm::linearRand<int>(0, (int)mTextures.size() - 1)]->index();
					}
				}
			}

			mRenderQueue = new RenderQueue();
			
			co_return;
		}

		void draw(GraphicsApi& cmd) override {

			float w = (float)mApp->getWidth();
			float h = (float)mApp->getHeight();
			float aspect = w / h;
			float ORTHO_SIZE = 100.0f;
			auto vp = glm::ortho(-ORTHO_SIZE * aspect, ORTHO_SIZE * aspect, ORTHO_SIZE, -ORTHO_SIZE, -100.0f, 100.0f);

			uint32_t perView = cmd.allocConstant(vp);

			mRenderQueue->clear();

			auto& renderCmds = mRenderQueue->getWriteCommands();
			for (auto& renderable : mRenderables) {
				uint32_t perObject = cmd.allocConstant(renderable.worldTransform);
				uint32_t perMaterial = cmd.allocConstant(renderable.texIndex);
				for (auto& prim : mMesh->renderPrimitives) {
					auto& rc = renderCmds.emplace_back();
					rc.renderPrimitive = prim;
					rc.pipelineState = mShader->pipelineState;
					rc.uniforms.set(perView, perObject, perMaterial);
				}
			}

			cmd.drawBatch(mRenderQueue);

		}
	};

	DEF_DEMO(RenderQueueDemo, "RenderQueue Demo");
}