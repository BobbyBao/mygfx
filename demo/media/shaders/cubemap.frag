#version 450

precision highp float;

#include <common_uniforms.glsl>
#include <common_texture_set.glsl>

#include <tonemapping.glsl>
layout(binding = 2) uniform MaterialUniforms {

};

//uniform float u_EnvIntensity;
//uniform float u_EnvBlurNormalized;
//uniform int u_MipCount;
//uniform samplerCube u_GGXEnvSampler;

layout(location = 0) in vec3 v_TexCoords;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 color = texCubeLod(u_GGXEnvSampler, v_TexCoords, u_EnvBlurNormalized * float(u_MipCount - 1)) * u_EnvIntensity;

#ifdef LINEAR_OUTPUT
    FragColor = color.rgba;
#else
    FragColor = vec4(toneMap(color.rgb), color.a);
#endif
}
