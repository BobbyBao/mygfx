#include "DemoApp.h"
#include "Mesh.h"
#include "Texture.h"

namespace mygfx::demo {
	
	class DescriptorSetDemo : public Demo {
	public:
		Ref<Mesh> mMesh;
		Ref<Shader> mShader;
		Vector<Ref<HwRenderPrimitive>> mPrimitives;

		void start() override {

			mMesh = Mesh::createCube();

			for (auto& subMesh : mMesh->getSubMeshes()) {
				mPrimitives.emplace_back(device().createRenderPrimitive(subMesh.vertexData, subMesh.drawArgs));
			}

			mShader = new Shader(vsCode, fsCode);
			mShader->setVertexInput({
				Format::R32G32B32_SFLOAT, Format::END,
				Format::R32G32_SFLOAT, Format::END,
				Format::R32G32B32_SFLOAT });

			auto& cmd = getGraphicsApi();

			mShader->updateDescriptorSet(0, 2, Texture::Green);

		}

		void draw(GraphicsApi& cmd) override {

			float aspect = ImGui::GetIO().DisplaySize.x / ImGui::GetIO().DisplaySize.y;
			auto vp = glm::ortho(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);

			uint32_t perView = device().allocConstant(vp);

			auto world = identity<mat4>();

			uint32_t perDraw = device().allocConstant(world);

			cmd.bindPipelineState(mShader->pipelineState);
			cmd.bindUniforms({ perView, perDraw});

			for (auto& prim : mPrimitives) {
				cmd.drawPrimitive(prim);
			}

		}


		const char* vsCode = R"(
			#version 450
			
			layout (binding = 0) uniform PerView {
				mat4 viewProj;
			} frameUniforms;

			layout (binding = 1) uniform PerRenderable {
				mat4 world;
			} objectUniforms;

			layout(location = 0) in vec3 inPos;
			layout(location = 1) in vec2 inUV;
			layout(location = 2) in vec3 inNorm;

			layout(location = 0) out vec2 outUV;
			layout(location = 1) out vec3 outNorm;

			void main()
			{
				outUV = inUV;
				outNorm = inNorm;
				gl_Position = frameUniforms.viewProj * objectUniforms.world * vec4(inPos.xyz, 1.0);
			}
		)";

		const char* fsCode = R"(
			#version 450

			#extension GL_EXT_nonuniform_qualifier : require

			layout(binding = 2) uniform sampler2D baseColorMap;

			layout(location = 0) in vec2 inUV;
			layout(location = 1) in vec3 inNorm;

			layout(location = 0) out vec4 outFragColor;

			void main()
			{
				outFragColor = texture(baseColorMap, inUV);
			}
		)";


	};

	DEF_DEMO(DescriptorSetDemo, "DescriptorSet Demo");
}