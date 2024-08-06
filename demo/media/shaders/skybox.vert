#version 450


#include <common_uniforms.glsl>

layout(location = 0) out vec3 v_TexCoords;

out gl_PerVertex
{
    vec4 gl_Position;
};

const vec4 FullScreenVertsPos[3] = { {-1, 1, 0, 1}, {3, 1, 0, 1}, {-1, -3, 0, 1} };
const vec2 FullScreenVertsUVs[3] = { { 0, 0}, {2, 0}, {0, 2} };

void main()
{
    gl_Position = FullScreenVertsPos[gl_VertexIndex];
    vec2 uv = FullScreenVertsUVs[gl_VertexIndex];
    vec4 clip = vec4(2 * uv.x - 1, 2 * uv.y, 0, 1);
    v_TexCoords = (u_InvViewProjectionMatrix * clip).xyz;
}