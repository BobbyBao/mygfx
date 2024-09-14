#ifndef _COMMON_VARYINGS_
#define _COMMON_VARYINGS_

#ifdef SHADER_STAGE_VERTEX
#define VARYING out
#elif defined(SHADER_STAGE_FRAGMENT)
#define VARYING in
#else
#define VARYING
#endif

layout(location = 0) VARYING vec3 v_Position;
layout(location = 1) VARYING vec2 v_texcoord_0;
layout(location = 2) VARYING vec2 v_texcoord_1;

#ifdef HAS_NORMAL_VEC3
#ifdef HAS_TANGENT_VEC4
layout(location = 3) VARYING mat3 v_TBN;
#else
layout(location = 3) VARYING vec3 v_Normal;
#endif
#endif

#ifdef HAS_COLOR_0_VEC3
layout(location = 6) VARYING vec3 v_Color;
#endif

#ifdef HAS_COLOR_0_VEC4
layout(location = 6) VARYING vec4 v_Color;
#endif


#endif