#include "ShaderLibs.h"

namespace mygfx::demo {

	static utils::Ref<Program> sColorShader;
	static utils::Ref<Program> sBasicShader;

	utils::Ref<Program> ShaderLibs::getColorShader() {
		if (sColorShader) {
			return sColorShader;
		}

		const char* vsCode = R"(
			#version 450
			
			layout (binding = 0) uniform ProjConstants {
				mat4 proj;
			} ubo;

			layout(location = 0) in vec3 inPos;
			layout(location = 1) in vec4 inColor;

			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = inColor;
				gl_Position = ubo.proj * vec4(inPos.xyz, 1.0);
			}
		)";

		const char* fsCode = R"(
			#version 450

			layout(location = 0) in vec4 inColor;
			layout(location = 0) out vec4 outFragColor;

			void main()
			{
				outFragColor = inColor;
			}
		)";

		sColorShader = new Program(vsCode, fsCode);
		sColorShader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::R8G8B8A8_UNORM});
		return sColorShader;
	}

	utils::Ref<Program> ShaderLibs::getBasicShader() {
		if (sBasicShader) {
			return sBasicShader;
		}

		const char* vsCode = R"(
			#version 450
			
			layout (binding = 0) uniform ProjConstants {
				mat4 proj;
			} ubo;

			layout(location = 0) in vec3 inPos;
			layout(location = 1) in vec4 inColor;
			layout(location = 2) in vec2 inUV;
			layout(location = 3) in int inTextureIndex;

			layout(location = 0) out vec2 outUV;
			layout(location = 1) out vec4 outColor;
			layout(location = 2) flat out int outTexIndex;

			void main()
			{
				outUV = inUV;
				outColor = inColor;
				outTexIndex = inTextureIndex;
				gl_Position = ubo.proj * vec4(inPos.xyz, 1.0);
			}
		)";

		const char* fsCode = R"(
			#version 450

			#extension GL_EXT_nonuniform_qualifier : require

			layout(set = 1, binding = 0) uniform sampler2D textures[];

			layout(location = 0) in vec2 inUV;
			layout(location = 1) in vec4 inColor;
			layout(location = 2) flat in int inTexIndex;

			layout(location = 0) out vec4 outFragColor;

			void main()
			{
				outFragColor = inColor*textureLod(textures[nonuniformEXT(inTexIndex)], inUV, 0);
			}
		)";

		sBasicShader = new Program(vsCode, fsCode);
		sBasicShader->setVertexInput({Format::R32G32B32_SFLOAT, Format::R8G8B8A8_UNORM, Format::R32G32_SFLOAT, Format::R32_SINT });
		return sBasicShader;
	}

	void ShaderLibs::clean() {
		sColorShader.reset();
		sBasicShader.reset();
	}
}