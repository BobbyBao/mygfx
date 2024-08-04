#ifndef _COMMON_UNIFORMS_
#define _COMMON_UNIFORMS_


layout(std140, binding = 0) uniform FrameUniforms {
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjectionMatrix;
    vec3 u_Camera;
    float pad1;

    float u_EnvIntensity;
    float u_Exposure;
    int u_MipCount;
    float pad2;
    
    int u_LambertianEnvTexture;
    int u_GGXEnvTexture;
    int u_GGXLUTTexture;
    float pad3;

    int u_CharlieEnvTexture;
    int u_CharlieLUTTexture;
    int u_SheenELUTTexture;
    float pad4;

    mat3 u_EnvRotation;
};

layout(std140, binding = 1) uniform ObjectUniforms {
    mat4 u_ModelMatrix;
    mat4 u_NormalMatrix;
};

layout(binding = 2) uniform MaterialUniforms {

#ifdef MATERIAL_METALLICROUGHNESS

    // Metallic Roughness
    vec4 u_BaseColorFactor;
    float u_MetallicFactor;
    float u_RoughnessFactor;
    vec2 pad1;

    int u_BaseColorTexture;
    int u_BaseColorUVSet;
    int u_MetallicRoughnessTexture;
    int u_MetallicRoughnessUVSet;

#ifdef HAS_BASECOLOR_UV_TRANSFORM
    mat3 u_BaseColorUVTransform;
#endif

#ifdef HAS_METALLICROUGHNESS_UV_TRANSFORM
    mat3 u_MetallicRoughnessUVTransform;
#endif

#endif

#ifdef MATERIAL_SPECULARGLOSSINESS

    // Specular Glossiness
    vec4 u_DiffuseFactor;
    vec3 u_SpecularFactor;
    float u_GlossinessFactor;

    //sampler2D 
    int u_DiffuseTexture;
    int u_DiffuseUVSet;
    int u_SpecularGlossinessTexture;
    int u_SpecularGlossinessUVSet;

#ifdef HAS_DIFFUSE_UV_TRANSFORM
    mat3 u_DiffuseUVTransform;
#endif

    //sampler2D 
#ifdef HAS_SPECULARGLOSSINESS_UV_TRANSFORM
    mat3 u_SpecularGlossinessUVTransform;
#endif

#endif

    // Sheen
    float u_SheenRoughnessFactor;
    vec3 u_SheenColorFactor;

    // Clearcoat
    float u_ClearcoatFactor;
    float u_ClearcoatRoughnessFactor;

    // Specular
    vec3 u_KHR_materials_specular_specularColorFactor;
    float u_KHR_materials_specular_specularFactor;

    // Transmission
    float u_TransmissionFactor;

    // Volume
    float u_ThicknessFactor;
    vec3 u_AttenuationColor;
    float u_AttenuationDistance;

    // Iridescence
    float u_IridescenceFactor;
    float u_IridescenceIor;
    float u_IridescenceThicknessMinimum;
    float u_IridescenceThicknessMaximum;

    // Diffuse Transmission
    float u_DiffuseTransmissionFactor;
    vec3 u_DiffuseTransmissionColorFactor;

    // Emissive Strength
    float u_EmissiveStrength;

    // IOR
    float u_Ior;

    // Anisotropy
    vec3 u_Anisotropy;

    // Dispersion
    float u_Dispersion;

    // Alpha mode
    float u_AlphaCutoff;

    #ifdef MATERIAL_TRANSMISSION
    ivec2 u_ScreenSize;
    #endif


    //sampler2D
    int u_NormalTexture;
    float u_NormalScale;
    int u_NormalUVSet;
#ifdef HAS_NORMAL_UV_TRANSFORM
    mat3 u_NormalUVTransform;
#endif

    vec3 u_EmissiveFactor;
    //sampler2D
    int u_EmissiveTexture;
    int u_EmissiveUVSet;
#ifdef HAS_EMISSIVE_UV_TRANSFORM
    mat3 u_EmissiveUVTransform;
#endif

    //sampler2D
    int u_OcclusionTexture;
    int u_OcclusionUVSet;
    float u_OcclusionStrength;
#ifdef HAS_OCCLUSION_UV_TRANSFORM
    mat3 u_OcclusionUVTransform;
#endif


};





#endif