#include "DemoApp.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"

namespace mygfx::demo {
	
	class AysncLoadDemo : public Demo {
	public:
		Ref<Mesh> mMesh;
		Ref<Shader> mShader;
		Vector<Ref<HwRenderPrimitive>> mPrimitives;

		void start() override {

			mShader = ShaderLibs::getSimpleLightShader();

			mMesh = Mesh::createCube();

		}

		void draw(GraphicsApi& cmd) override {

			float w = (float)mApp->getWidth();
			float h = (float)mApp->getHeight();
			float aspect = w / h;
			auto vp = glm::ortho(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);

			uint32_t perView = device().allocConstant(vp);

			auto world = identity<mat4>();

			uint32_t perDraw = device().allocConstant(world);
			uint32_t perMaterial = device().allocConstant(Texture::Red->index());

			cmd.bindPipelineState(mShader->pipelineState);
			cmd.bindUniforms({ perView, perDraw, perMaterial });

			for (auto& prim : mMesh->renderPrimitives) {
				cmd.drawPrimitive(prim);
			}

		}
	};

	DEF_DEMO(AysncLoadDemo, "Aysnc Load Demo");	
}