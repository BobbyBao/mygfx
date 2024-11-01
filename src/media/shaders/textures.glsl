// IBL

#include <common_texture_set.glsl>


layout(std140, binding = 2) uniform MaterialUniforms {

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

#ifdef HAS_SPECULARGLOSSINESS_UV_TRANSFORM
    mat3 u_SpecularGlossinessUVTransform;
#endif

#endif


    //sampler2D
    int u_NormalTexture;
    float u_NormalScale;
    int u_NormalUVSet;
    float pad2;
#ifdef HAS_NORMAL_UV_TRANSFORM
    mat3 u_NormalUVTransform;
#endif

    vec3 u_EmissiveFactor;
    int u_EmissiveTexture;
    int u_EmissiveUVSet;
    int u_OcclusionTexture;
    int u_OcclusionUVSet;
    float u_OcclusionStrength;

#ifdef HAS_EMISSIVE_UV_TRANSFORM
    mat3 u_EmissiveUVTransform;
#endif

#ifdef HAS_OCCLUSION_UV_TRANSFORM
    mat3 u_OcclusionUVTransform;
#endif

    // Sheen
    float u_SheenRoughnessFactor;
    vec3 u_SheenColorFactor;

    // Clearcoat
    float u_ClearcoatFactor;
    float u_ClearcoatRoughnessFactor;

    // Transmission
    float u_TransmissionFactor;

    // Volume
    float u_ThicknessFactor;
    vec3 u_AttenuationColor;
    float u_AttenuationDistance;
    
    // Specular
    vec3 u_KHR_materials_specular_specularColorFactor;
    float u_KHR_materials_specular_specularFactor;

    // Iridescence
    float u_IridescenceFactor;
    float u_IridescenceIor;
    float u_IridescenceThicknessMinimum;
    float u_IridescenceThicknessMaximum;

    // Diffuse Transmission
    vec3 u_DiffuseTransmissionColorFactor;
    float u_DiffuseTransmissionFactor;

    // Anisotropy
    vec3 u_Anisotropy;

    // Emissive Strength
    float u_EmissiveStrength;

    // IOR
    float u_Ior;

    // Dispersion
    float u_Dispersion;

    // Alpha mode
    float u_AlphaCutoff;

    //float pad3;
};


#define u_LambertianEnvSampler textures_Cube[nonuniformEXT(u_LambertianEnvTexture)]
#define u_GGXEnvSampler textures_Cube[nonuniformEXT(u_GGXEnvTexture)]
#define u_GGXLUT textures_2d[nonuniformEXT(u_GGXLUTTexture)]

#define u_CharlieEnvSampler textures_Cube[nonuniformEXT(u_CharlieEnvTexture)]
#define u_CharlieLUT textures_2d[nonuniformEXT(u_CharlieLUTTexture)]
#define u_SheenELUT textures_2d[nonuniformEXT(u_SheenELUTTexture)]

#define u_NormalSampler textures_2d[nonuniformEXT(u_NormalTexture)]
#define u_EmissiveSampler textures_2d[nonuniformEXT(u_EmissiveTexture)]
#define u_OcclusionSampler textures_2d[nonuniformEXT(u_OcclusionTexture)]

#ifdef MATERIAL_METALLICROUGHNESS
#define u_BaseColorSampler textures_2d[nonuniformEXT(u_BaseColorTexture)]
#define u_MetallicRoughnessSampler textures_2d[nonuniformEXT(u_MetallicRoughnessTexture)]
#endif

#ifdef MATERIAL_SPECULARGLOSSINESS
#define u_DiffuseSampler textures_2d[nonuniformEXT(u_DiffuseTexture)]
#define u_SpecularGlossinessSampler textures_2d[nonuniformEXT(u_SpecularGlossinessTexture)]
#endif


vec2 getNormalUV()
{
    vec3 uv = vec3(u_NormalUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);

#ifdef HAS_NORMAL_UV_TRANSFORM
    uv = u_NormalUVTransform * uv;
#endif

    return uv.xy;
}


vec2 getEmissiveUV()
{
    vec3 uv = vec3(u_EmissiveUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);

#ifdef HAS_EMISSIVE_UV_TRANSFORM
    uv = u_EmissiveUVTransform * uv;
#endif

    return uv.xy;
}


vec2 getOcclusionUV()
{
    vec3 uv = vec3(u_OcclusionUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);

#ifdef HAS_OCCLUSION_UV_TRANSFORM
    uv = u_OcclusionUVTransform * uv;
#endif

    return uv.xy;
}


// Metallic Roughness Material


#ifdef MATERIAL_METALLICROUGHNESS

vec2 getBaseColorUV()
{
    vec3 uv = vec3(u_BaseColorUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);

#ifdef HAS_BASECOLOR_UV_TRANSFORM
    uv = u_BaseColorUVTransform * uv;
#endif

    return uv.xy;
}

vec2 getMetallicRoughnessUV()
{
    vec3 uv = vec3(u_MetallicRoughnessUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);

#ifdef HAS_METALLICROUGHNESS_UV_TRANSFORM
    uv = u_MetallicRoughnessUVTransform * uv;
#endif

    return uv.xy;
}

#endif


// Specular Glossiness Material


#ifdef MATERIAL_SPECULARGLOSSINESS

vec2 getSpecularGlossinessUV()
{
    vec3 uv = vec3(u_SpecularGlossinessUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);

#ifdef HAS_SPECULARGLOSSINESS_UV_TRANSFORM
    uv = u_SpecularGlossinessUVTransform * uv;
#endif

    return uv.xy;
}

vec2 getDiffuseUV()
{
    vec3 uv = vec3(u_DiffuseUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);

#ifdef HAS_DIFFUSE_UV_TRANSFORM
    uv = u_DiffuseUVTransform * uv;
#endif

    return uv.xy;
}

#endif


// Clearcoat Material


#ifdef MATERIAL_CLEARCOAT

uniform sampler2D u_ClearcoatSampler;
uniform int u_ClearcoatUVSet;
uniform mat3 u_ClearcoatUVTransform;

uniform sampler2D u_ClearcoatRoughnessSampler;
uniform int u_ClearcoatRoughnessUVSet;
uniform mat3 u_ClearcoatRoughnessUVTransform;

uniform sampler2D u_ClearcoatNormalSampler;
uniform int u_ClearcoatNormalUVSet;
uniform mat3 u_ClearcoatNormalUVTransform;
uniform float u_ClearcoatNormalScale;


vec2 getClearcoatUV()
{
    vec3 uv = vec3(u_ClearcoatUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_CLEARCOAT_UV_TRANSFORM
    uv = u_ClearcoatUVTransform * uv;
#endif
    return uv.xy;
}

vec2 getClearcoatRoughnessUV()
{
    vec3 uv = vec3(u_ClearcoatRoughnessUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_CLEARCOATROUGHNESS_UV_TRANSFORM
    uv = u_ClearcoatRoughnessUVTransform * uv;
#endif
    return uv.xy;
}

vec2 getClearcoatNormalUV()
{
    vec3 uv = vec3(u_ClearcoatNormalUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_CLEARCOATNORMAL_UV_TRANSFORM
    uv = u_ClearcoatNormalUVTransform * uv;
#endif
    return uv.xy;
}

#endif


// Sheen Material


#ifdef MATERIAL_SHEEN

uniform sampler2D u_SheenColorSampler;
uniform int u_SheenColorUVSet;
uniform mat3 u_SheenColorUVTransform;
uniform sampler2D u_SheenRoughnessSampler;
uniform int u_SheenRoughnessUVSet;
uniform mat3 u_SheenRoughnessUVTransform;


vec2 getSheenColorUV()
{
    vec3 uv = vec3(u_SheenColorUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_SHEENCOLOR_UV_TRANSFORM
    uv = u_SheenColorUVTransform * uv;
#endif
    return uv.xy;
}

vec2 getSheenRoughnessUV()
{
    vec3 uv = vec3(u_SheenRoughnessUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_SHEENROUGHNESS_UV_TRANSFORM
    uv = u_SheenRoughnessUVTransform * uv;
#endif
    return uv.xy;
}

#endif


// Specular Material


#ifdef MATERIAL_SPECULAR

uniform sampler2D u_SpecularSampler;
uniform int u_SpecularUVSet;
uniform mat3 u_SpecularUVTransform;
uniform sampler2D u_SpecularColorSampler;
uniform int u_SpecularColorUVSet;
uniform mat3 u_SpecularColorUVTransform;


vec2 getSpecularUV()
{
    vec3 uv = vec3(u_SpecularUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_SPECULAR_UV_TRANSFORM
    uv = u_SpecularUVTransform * uv;
#endif
    return uv.xy;
}

vec2 getSpecularColorUV()
{
    vec3 uv = vec3(u_SpecularColorUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_SPECULARCOLOR_UV_TRANSFORM
    uv = u_SpecularColorUVTransform * uv;
#endif
    return uv.xy;
}

#endif


// Transmission Material


#ifdef MATERIAL_TRANSMISSION

uniform sampler2D u_TransmissionSampler;
uniform int u_TransmissionUVSet;
uniform mat3 u_TransmissionUVTransform;
uniform sampler2D u_TransmissionFramebufferSampler;
uniform ivec2 u_TransmissionFramebufferSize;


vec2 getTransmissionUV()
{
    vec3 uv = vec3(u_TransmissionUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_TRANSMISSION_UV_TRANSFORM
    uv = u_TransmissionUVTransform * uv;
#endif
    return uv.xy;
}

#endif


// Volume Material


#ifdef MATERIAL_VOLUME

uniform sampler2D u_ThicknessSampler;
uniform int u_ThicknessUVSet;
uniform mat3 u_ThicknessUVTransform;


vec2 getThicknessUV()
{
    vec3 uv = vec3(u_ThicknessUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_THICKNESS_UV_TRANSFORM
    uv = u_ThicknessUVTransform * uv;
#endif
    return uv.xy;
}

#endif


// Iridescence


#ifdef MATERIAL_IRIDESCENCE

uniform sampler2D u_IridescenceSampler;
uniform int u_IridescenceUVSet;
uniform mat3 u_IridescenceUVTransform;

uniform sampler2D u_IridescenceThicknessSampler;
uniform int u_IridescenceThicknessUVSet;
uniform mat3 u_IridescenceThicknessUVTransform;


vec2 getIridescenceUV()
{
    vec3 uv = vec3(u_IridescenceUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_IRIDESCENCE_UV_TRANSFORM
    uv = u_IridescenceUVTransform * uv;
#endif
    return uv.xy;
}

vec2 getIridescenceThicknessUV()
{
    vec3 uv = vec3(u_IridescenceThicknessUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_IRIDESCENCETHICKNESS_UV_TRANSFORM
    uv = u_IridescenceThicknessUVTransform * uv;
#endif
    return uv.xy;
}

#endif


// Diffuse Transmission

#ifdef MATERIAL_DIFFUSE_TRANSMISSION

uniform sampler2D u_DiffuseTransmissionSampler;
uniform int u_DiffuseTransmissionUVSet;
uniform mat3 u_DiffuseTransmissionUVTransform;

uniform sampler2D u_DiffuseTransmissionColorSampler;
uniform int u_DiffuseTransmissionColorUVSet;
uniform mat3 u_DiffuseTransmissionColorUVTransform;


vec2 getDiffuseTransmissionUV()
{
    vec3 uv = vec3(u_DiffuseTransmissionUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_DIFFUSE_TRANSMISSION_UV_TRANSFORM
    uv = u_DiffuseTransmissionUVTransform * uv;
#endif
    return uv.xy;
}

vec2 getDiffuseTransmissionColorUV()
{
    vec3 uv = vec3(u_DiffuseTransmissionColorUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_DIFFUSE_TRANSMISSION_COLOR_UV_TRANSFORM
    uv = u_DiffuseTransmissionColorUVTransform * uv;
#endif
    return uv.xy;
}

#endif

// Anisotropy

#ifdef MATERIAL_ANISOTROPY

uniform sampler2D u_AnisotropySampler;
uniform int u_AnisotropyUVSet;
uniform mat3 u_AnisotropyUVTransform;

vec2 getAnisotropyUV()
{
    vec3 uv = vec3(u_AnisotropyUVSet < 1 ? v_texcoord_0 : v_texcoord_1, 1.0);
#ifdef HAS_ANISOTROPY_UV_TRANSFORM
    uv = u_AnisotropyUVTransform * uv;
#endif
    return uv.xy;
}

#endif
