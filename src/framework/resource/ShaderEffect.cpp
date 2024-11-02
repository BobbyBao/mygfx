#include "ShaderEffect.h"
#include "Shader.h"

namespace mygfx {

ShaderEffect::ShaderEffect()
{
}

ShaderEffect::~ShaderEffect()
{
}

Shader* ShaderEffect::getShader(const String& name)
{
    for (auto& shader : mShaders) {
        if (shader->getName() == name) {
            return shader;
        }
    }
    return nullptr;
}

Shader* ShaderEffect::getShader(const PassID& pass)
{
    return mShaderPasses[pass.index];
}

void ShaderEffect::add(const String& vsCode, const String& fsCode, const DefineList* marcos)
{
    add(new Shader(vsCode, fsCode, marcos));
}

void ShaderEffect::add(const String& csCode, const DefineList* marcos)
{
    add(new Shader(csCode, marcos));
}

void ShaderEffect::add(Shader* shader)
{
    mShaders.emplace_back(shader);
    mShaderPasses[shader->passID.index] = shader;
}

Ref<ShaderEffect> ShaderEffect::fromFile(const String& vs, const String& fs, const DefineList* marcos)
{
    Ref<Shader> shader(new Shader());
    shader->loadShader(vs, fs, marcos);
    Ref<ShaderEffect> fx(new ShaderEffect());
    fx->add(shader);
    return fx;
}
}