#version 450

#include <common_uniforms.glsl>

layout(location = 0) in vec3 a_position;

layout(location = 0) out vec3 v_TexCoords;


void main()
{
    v_TexCoords = u_EnvRotation * a_position;
    v_TexCoords.y = 1.0 - v_TexCoords.y;
    mat4 mat = u_ViewProjectionMatrix;
    mat[3] = vec4(0.0, 0.0, 0.0, 0.1);
    vec4 pos = mat * vec4(a_position, 1.0);
    gl_Position = pos.xyww;
}
