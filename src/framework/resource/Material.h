#pragma once
#include "PipelineState.h"
#include "core/Fwd.h"
#include "core/Maths.h"
#include "core/Resource.h"
#include "resource/ShaderEffect.h"
#include <variant>

namespace mygfx {

class Shader;
class ShaderEffect;
class ShaderResourceInfo;
class Texture;

using ShaderParameter = std::variant<int, float, vec2, vec3, vec4, Ref<Texture>>;

class Material : public Resource {
public:
    Material();
    Material(ShaderEffect* shader, const String& materialUniformName);
    ~Material();

    void setShader(ShaderEffect* shader, const String& materialUniformName);

    void setShaderParameter(const String& name, int v);
    void setShaderParameter(const String& name, float v);
    void setShaderParameter(const String& name, const vec3& v);
    void setShaderParameter(const String& name, const vec4& v);
    void setShaderParameter(const String& name, Texture* tex);

    void setDoubleSide(bool v);
    void setWireframe(bool v);
    void setBlendMode(BlendMode blendMode);

    Shader* shader() { return mShaderFX->getMainPass(); }
    const PipelineState& getPipelineState() const { return mPipelineState; }

    inline uint32_t getMaterialUniforms() const
    {
        return mMaterialUniforms;
    }

    static void updateAll();

protected:
    void update();
    Ref<ShaderEffect> mShaderFX;
    PipelineState mPipelineState;
    String mMaterialUniformName;
    Ref<ShaderResourceInfo> mShaderResourceInfo;
    ByteArray mMaterialData;
    HashMap<String, ShaderParameter> mShaderParameters;
    mutable uint32_t mMaterialUniforms = INVALID_UNIFORM_OFFSET;
};

}