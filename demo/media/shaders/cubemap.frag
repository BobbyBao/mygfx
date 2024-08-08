#version 450

precision highp float;

#include <common_uniforms.glsl>
#include <common_texture_set.glsl>

#include <tonemapping.glsl>

layout(std140, binding = 2) uniform MaterialUniforms {
    int u_MipLevel;   
    float u_EnvBlurNormalized;
};

layout(location = 0) in vec3 v_TexCoords;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 color = texCubeLod(u_GGXEnvTexture, v_TexCoords, u_EnvBlurNormalized * float(u_MipLevel)) * u_EnvIntensity;

#ifdef LINEAR_OUTPUT
    FragColor = color.rgba;
#else
    FragColor = vec4(toneMap(color.rgb), color.a);
#endif
}
