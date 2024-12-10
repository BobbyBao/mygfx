#include "ShaderLibs.h"

namespace mygfx::samples {

static utils::Ref<Shader> sColorShader;
static utils::Ref<Shader> sUnlitShader;
static utils::Ref<Shader> sLightShader;
static utils::Ref<Shader> sFullscreenShader;

void ShaderLibs::clean()
{
    sColorShader.reset();
    sUnlitShader.reset();
    sLightShader.reset();
    sFullscreenShader.reset();
}

utils::Ref<Shader> ShaderLibs::getColorShader()
{
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

    sColorShader = new Shader(vsCode, fsCode);
    sColorShader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::R8G8B8A8_UNORM });
    return sColorShader;
}

utils::Ref<Shader> ShaderLibs::getUnlitShader()
{
    if (sUnlitShader) {
        return sUnlitShader;
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

    sUnlitShader = new Shader(vsCode, fsCode);
    sUnlitShader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::R8G8B8A8_UNORM, Format::R32G32_SFLOAT, Format::R32_SINT });
    sUnlitShader->setBlendMode(BlendMode::ALPHA);
    sUnlitShader->setCullMode(CullMode::NONE);
    sUnlitShader->setDepthTest(false, false);
    return sUnlitShader;
}

utils::Ref<Shader> ShaderLibs::getSimpleLightShader()
{
    if (sLightShader) {
        return sLightShader;
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

	layout (binding = 2) uniform MaterialUniforms {
		int baseColor;
	} materialUniforms;

	layout(set = 1, binding = 0) uniform sampler2D textures[];

	layout(location = 0) in vec2 inUV;
	layout(location = 1) in vec3 inNorm;

	layout(location = 0) out vec4 outFragColor;

	void main()
	{
		outFragColor = texture(textures[nonuniformEXT(materialUniforms.baseColor)], inUV);
	}
	)";

    sLightShader = new Shader(vsCode, fsCode);
    sLightShader->setVertexInput({ Format::R32G32B32_SFLOAT, {},
        Format::R32G32_SFLOAT, {},
        Format::R32G32B32_SFLOAT });
    return sLightShader;
}

Shader* ShaderLibs::getFullscreenShader()
{
    if (sFullscreenShader) {
        return sFullscreenShader;
    }

    const char* vsCode = R"(
#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec2 out_UV;
layout(location = 1) flat out int out_TexIndex;

out gl_PerVertex
{
    vec4 gl_Position;
};

const vec4 FullScreenVertsPos[3] = { {-1, 1, 1, 1}, {3, 1, 1, 1}, {-1, -3, 1, 1} };
const vec2 FullScreenVertsUVs[3] = { { 0, 0}, {2, 0}, {0, 2} };

void main()
{
    gl_Position = FullScreenVertsPos[gl_VertexIndex];
    out_UV = FullScreenVertsUVs[gl_VertexIndex];
    out_TexIndex = gl_InstanceIndex;
}
	)";

    const char* fsCode = R"(
#version 450

#extension GL_EXT_nonuniform_qualifier : require
    
layout(set = 0, binding = 0) uniform sampler2D textures_2d[];

layout(location = 0) in vec2 in_UV;
layout(location = 1) flat in int in_TexIndex;
layout(location = 0) out vec4 out_FragColor;

void main()
{
    out_FragColor = texture(textures_2d[nonuniformEXT(in_TexIndex)], in_UV);
}
	)";

    sFullscreenShader = new Shader(vsCode, fsCode);
    sFullscreenShader->setVertexInput({});
	sFullscreenShader->setCullMode(CullMode::NONE);
    sFullscreenShader->setDepthTest(false, false);
    return sFullscreenShader;
}
}