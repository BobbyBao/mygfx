#ifndef _COMMON_UNIFORMS_
#define _COMMON_UNIFORMS_

#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : enable

layout(std140, binding = 0) uniform FrameUniforms {
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjectionMatrix;
    mat4 u_InvViewMatrix;
    mat4 u_InvProjectionMatrix;
    mat4 u_InvViewProjectionMatrix;

    vec3 u_Camera;
    float nearZ;
    vec3 u_CameraDir;
    float farZ;

    ivec2 u_ScreenSize;
    vec2 u_InvScreenSize;

    float u_EnvIntensity;
    float u_Exposure;
    int u_MipCount;
    float pad1_0;
    
    int u_LambertianEnvTexture;
    int u_GGXEnvTexture;
    int u_GGXLUTTexture;
    float pad3_0;

    int u_CharlieEnvTexture;
    int u_CharlieLUTTexture;
    int u_SheenELUTTexture;
    float pad4_0;

    mat3 u_EnvRotation;
};

layout(std140, binding = 1) uniform ObjectUniforms {
    mat4 u_ModelMatrix;
    mat4 u_NormalMatrix;
};




#endif