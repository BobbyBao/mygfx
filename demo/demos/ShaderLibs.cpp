#include "ShaderLibs.h"

namespace mygfx::demo {
	static utils::Ref<Program> sBasicShader;

	utils::Ref<Program> ShaderLibs::getBasicShader() {
		if (sBasicShader) {
			return sBasicShader;
		}

		const char* vsCode = R"(
			#version 450

			#extension GL_ARB_separate_shader_objects : enable
			#extension GL_ARB_shading_language_420pack : enable

			layout (location = 0) in vec4 inPos;
			layout (location = 1) in vec2 inUV;
			layout (location = 2) in vec4 inColor;

			layout (binding = 0) uniform ProjConstants {
				mat4 proj;
			} ubo;

			layout (location = 0) out vec2 outUV;
			layout (location = 1) out vec4 outColor;
			layout (location = 2) flat out int texIndex;

			out gl_PerVertex 
			{
				vec4 gl_Position;
			};

			void main() 
			{
				outUV = inUV;
				outColor = inColor;
				gl_Position = ubo.proj * inPos;// vec4(inPos, 0.0, 1.0);
				texIndex = gl_InstanceIndex;
			}
		)";

		const char* fsCode = R"(
			#version 450

			#extension GL_ARB_separate_shader_objects : enable
			#extension GL_ARB_shading_language_420pack : enable
			#extension GL_EXT_nonuniform_qualifier : require


			layout (location = 0) in vec2 inUV;
			layout (location = 1) in vec4 inColor;

			layout(location = 2) flat in int texIndex;

			layout (location = 0) out vec4 outColor;

			layout(set = 1, binding = 0) uniform sampler2D textures[];

			void main() 
			{
				outColor = inColor;
				
				if (texIndex < 0) {
					outColor.a *= textureLod(textures[nonuniformEXT(-texIndex)], inUV, 0).r;
				} else {
					outColor *= textureLod(textures[nonuniformEXT(texIndex)], inUV, 0);
				}
			}
		)";

		sBasicShader = new Program(vsCode, fsCode);
		sBasicShader->setVertexInput({Format::R32G32B32_SFLOAT, Format::R32G32_SFLOAT, Format::R8G8B8A8_UNORM });

		return sBasicShader;
	}
}