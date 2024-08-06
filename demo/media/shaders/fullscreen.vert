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