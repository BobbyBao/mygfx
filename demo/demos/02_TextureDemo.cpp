#include "DemoApp.h"
#include "Mesh.h"
#include "Texture.h"

namespace mygfx::demo {
	
	class TextureDemo : public Demo {
	public:
		Ref<Mesh> mMesh;
		Ref<Program> mShader;
		Vector<Ref<HwRenderPrimitive>> mPrimitives;

		struct VertexColored {
			float3 pos;
			uint32_t color;
		};

		void start() override {

			mShader = ShaderLibs::getSimpleLightShader();

			mMesh = Mesh::createCube();
			for (auto& subMesh : mMesh->getSubMeshes()) {
				mPrimitives.emplace_back( device().createRenderPrimitive(subMesh.vertexData, subMesh.drawArgs));
			}

		}

		void draw(GraphicsApi& cmd) override {

			float aspect = ImGui::GetIO().DisplaySize.x / ImGui::GetIO().DisplaySize.y;
			float L = -aspect;
			float R = aspect;
			float B = 1;
			float T = -1.0f;
			float vp[4][4] =
			{
				{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
				{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
				{ 0.0f,         0.0f,           0.5f,       0.0f },
				{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
			};

			uint32_t perView = device().allocConstant(vp);
			float m[4][4] =
			{
				{ 1.0f, 0.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f,	1.0f },
			};

			uint32_t perDraw = device().allocConstant(m);		
			uint32_t perMaterial = device().allocConstant(Texture::Red->index());

			cmd.bindPipelineState(mShader->pipelineState);
			cmd.bindUniforms({ perView, perDraw, perMaterial });

			for (auto& prim : mPrimitives) {
				cmd.drawPrimitive(prim);
			}

		}
	};

	DEF_DEMO(TextureDemo, "Texture demo");	
}