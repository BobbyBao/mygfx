#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_scalar_block_layout : require

#include <common_uniforms.glsl>

#include <animation.glsl>

layout(location = 0) in vec3 a_position;

#ifdef HAS_NORMAL_VEC3
layout(location = HAS_NORMAL_VEC3) in vec3 a_normal;

#ifdef HAS_TANGENT_VEC4
layout(location = HAS_TANGENT_VEC4) in vec4 a_tangent;
#endif

#endif

#ifdef HAS_TEXCOORD_0_VEC2
layout(location = HAS_TEXCOORD_0_VEC2) in vec2 a_texcoord_0;
#endif

#ifdef HAS_TEXCOORD_1_VEC2
layout(location = HAS_TEXCOORD_1_VEC2) in vec2 a_texcoord_1;
#endif

#ifdef HAS_COLOR_0_VEC3
layout(location = HAS_COLOR_0_VEC3) in vec3 a_color_0;
#endif

#ifdef HAS_COLOR_0_VEC4
layout(location = HAS_COLOR_0_VEC4) in vec4 a_color_0;
#endif

#ifdef HAS_JOINTS_0_VEC4
layout(location = HAS_JOINTS_0_VEC4) in vec4 a_joints_0;
#endif

#ifdef HAS_JOINTS_1_VEC4
layout(location = HAS_JOINTS_1_VEC4) in vec4 a_joints_1;
#endif

#ifdef HAS_WEIGHTS_0_VEC4
layout(location = HAS_WEIGHTS_0_VEC4) in vec4 a_weights_0;
#endif

#ifdef HAS_WEIGHTS_1_VEC4
layout(location = HAS_WEIGHTS_1_VEC4) in vec4 a_weights_1;
#endif

#include <common_varyings.glsl>


vec4 getPosition()
{
    vec4 pos = vec4(a_position, 1.0);

#ifdef USE_MORPHING
    pos += getTargetPosition(gl_VertexID);
#endif

#ifdef USE_SKINNING
    pos = getSkinningMatrix() * pos;
#endif

    return pos;
}


#ifdef HAS_NORMAL_VEC3
vec3 getNormal()
{
    vec3 normal = a_normal;

#ifdef USE_MORPHING
    normal += getTargetNormal(gl_VertexID);
#endif

#ifdef USE_SKINNING
    normal = mat3(getSkinningNormalMatrix()) * normal;
#endif

    return normalize(normal);
}
#endif

#ifdef HAS_NORMAL_VEC3
#ifdef HAS_TANGENT_VEC4
vec3 getTangent()
{
    vec3 tangent = a_tangent.xyz;

#ifdef USE_MORPHING
    tangent += getTargetTangent(gl_VertexID);
#endif

#ifdef USE_SKINNING
    tangent = mat3(getSkinningMatrix()) * tangent;
#endif

    return normalize(tangent);
}
#endif
#endif


void main()
{
    gl_PointSize = 1.0f;
#ifdef USE_INSTANCING
    vec4 pos = u_ModelMatrix * u_TransformBuffer.m[gl_InstanceIndex] * getPosition();
#else
    vec4 pos = u_ModelMatrix * getPosition();
#endif
    v_Position = vec3(pos.xyz) / pos.w;

#ifdef HAS_NORMAL_VEC3
#ifdef HAS_TANGENT_VEC4
    vec3 tangent = getTangent();
    vec3 normalW = normalize(u_NormalMatrix * getNormal());
    vec3 tangentW = normalize(vec3(u_ModelMatrix * vec4(tangent, 0.0)));
    vec3 bitangentW = cross(normalW, tangentW) * a_tangent.w;
    v_TBN = mat3(tangentW, bitangentW, normalW);
#else
    v_Normal = normalize(u_NormalMatrix * getNormal());
#endif
#endif

    v_texcoord_0 = vec2(0.0, 0.0);
    v_texcoord_1 = vec2(0.0, 0.0);

#ifdef HAS_TEXCOORD_0_VEC2
    v_texcoord_0 = a_texcoord_0;
#endif

#ifdef HAS_TEXCOORD_1_VEC2
    v_texcoord_1 = a_texcoord_1;
#endif

#ifdef USE_MORPHING
    v_texcoord_0 += getTargetTexCoord0(gl_VertexID);
    v_texcoord_1 += getTargetTexCoord1(gl_VertexID);
#endif


#if defined(HAS_COLOR_0_VEC3) 
    v_Color = a_color_0;
#if defined(USE_MORPHING)
    v_Color = clamp(v_Color + getTargetColor0(gl_VertexID).xyz, 0.0f, 1.0f);
#endif
#endif

#if defined(HAS_COLOR_0_VEC4) 
    v_Color = a_color_0;
#if defined(USE_MORPHING)
    v_Color = clamp(v_Color + getTargetColor0(gl_VertexID), 0.0f, 1.0f);
#endif
#endif

    gl_Position = u_ViewProjectionMatrix * pos;
}
