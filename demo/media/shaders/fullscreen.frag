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