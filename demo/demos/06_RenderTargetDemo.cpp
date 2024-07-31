#include "DemoApp.h"
#include "Mesh.h"
#include "Texture.h"

namespace mygfx::demo {
	
	class RenderTargetDemo : public Demo {
	public:
		Ref<Mesh> mMesh;
		Ref<Program> mShader;
		Vector<Ref<HwRenderPrimitive>> mPrimitives;

		void start() override {

			mShader = ShaderLibs::getSimpleLightShader();

			mMesh = Mesh::createCube();
			for (auto& subMesh : mMesh->getSubMeshes()) {
				mPrimitives.emplace_back( device().createRenderPrimitive(subMesh.vertexData, subMesh.drawArgs));
			}

		}

		void draw(GraphicsApi& cmd) override {

			float aspect = ImGui::GetIO().DisplaySize.x / ImGui::GetIO().DisplaySize.y;
			auto vp = glm::ortho(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);

			uint32_t perView = device().allocConstant(vp);

			auto world = identity<mat4>();

			uint32_t perDraw = device().allocConstant(world);
			uint32_t perMaterial = device().allocConstant(Texture::Red->index());

			cmd.bindPipelineState(mShader->pipelineState);
			cmd.bindUniforms({ perView, perDraw, perMaterial });

			for (auto& prim : mPrimitives) {
				cmd.drawPrimitive(prim);
			}

		}
	};

	DEF_DEMO(RenderTargetDemo, "RenderTarget Demo");	
}