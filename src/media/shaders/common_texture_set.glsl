#ifndef _COMMON_TEXTURE_SET_
#define _COMMON_TEXTURE_SET_

#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : require

#ifndef TEXTURE_SET
#define TEXTURE_SET 1
#endif



layout(set = TEXTURE_SET, binding = 0) uniform sampler1D textures_1d[];
layout(set = TEXTURE_SET, binding = 0) uniform isampler1D textures_i1d[];
layout(set = TEXTURE_SET, binding = 0) uniform usampler1D textures_u1d[];
layout(set = TEXTURE_SET, binding = 0) uniform sampler2D textures_2d[];
layout(set = TEXTURE_SET, binding = 0) uniform isampler2D textures_i2d[];
layout(set = TEXTURE_SET, binding = 0) uniform usampler2D textures_u2d[];
layout(set = TEXTURE_SET, binding = 0) uniform sampler3D textures_3d[];
layout(set = TEXTURE_SET, binding = 0) uniform isampler3D textures_i3d[];
layout(set = TEXTURE_SET, binding = 0) uniform usampler3D textures_u3d[];
layout(set = TEXTURE_SET, binding = 0) uniform samplerCube textures_Cube[];
layout(set = TEXTURE_SET, binding = 0) uniform sampler2DShadow textures_Shadow[];
layout(set = TEXTURE_SET, binding = 0) uniform sampler2DArray textures_2dArray[];
layout(set = TEXTURE_SET, binding = 0) uniform isampler2DArray textures_i2dArray[];
layout(set = TEXTURE_SET, binding = 0) uniform usampler2DArray textures_u2dArray[];
layout(set = TEXTURE_SET, binding = 0) uniform sampler2DArrayShadow textures_ArrayShadow[];


vec4 tex1D(int tex, float texCoord)
{
	return texture(textures_1d[nonuniformEXT(tex)], texCoord);
}

vec4 tex2D(int tex, vec2 texCoord)
{
	return texture(textures_2d[nonuniformEXT(tex)], texCoord);
}

vec4 tex3D(int tex, vec3 texCoord)
{
	return texture(textures_3d[nonuniformEXT(tex)], texCoord);
}

vec4 texCube(int tex, vec3 texCoord)
{
	return texture(textures_Cube[nonuniformEXT(tex)], texCoord);
}

vec4 texCubeLod(int tex, vec3 texCoord, float lod)
{
	return textureLod(textures_Cube[nonuniformEXT(tex)], texCoord, lod);
}

float texShadow(int tex, vec3 texCoord)
{
	return texture(textures_Shadow[nonuniformEXT(tex)], texCoord);
}

#endif