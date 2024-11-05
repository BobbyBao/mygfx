#include "ShaderEffect.h"
#include "Shader.h"
#include "core/Config.h"
#include "core/FileSystem.h"
#include "utils/Log.h"

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

static bool loadShader(Shader* shader, const ConfigValue& shaderNode)
{
    if (auto cs = shaderNode.find("ComputeShader")) {
        shader->loadShader(cs->getString());
    }

    if (auto vs = shaderNode.find("VertexShader")) {
        if (auto fs = shaderNode.find("PixelShader")) {
            shader->loadShader(vs->getString(), fs->getString());
        }
    }

    if (auto cs = shaderNode.find("@ComputeShader")) {
        shader->addShader(ShaderStage::COMPUTE, "", cs->getString());
    }

    if (auto vs = shaderNode.find("@VertexShader")) {
        if (auto fs = shaderNode.find("@PixelShader")) {
            shader->addShader(ShaderStage::VERTEX, "", vs->getString());
            shader->addShader(ShaderStage::FRAGMENT, "", fs->getString());
        }
    }

    if (auto vertexInput = shaderNode.find("VertexInput")) {
    }

    return true;
}

static Ref<Shader> loadShader(const ConfigValue& shaderNode)
{
    Ref<Shader> shader(new Shader());
    if (!loadShader(shader, shaderNode)) {
        return nullptr;
    }

    shader->setName(shaderNode.name);
    return shader;
}

Ref<ShaderEffect> ShaderEffect::load(const String& fileName)
{
    auto text = FileSystem::readAllText(fileName);

    Config cfg;
    if (!cfg.parse(text)) {
        return nullptr;
    }

    auto fxNode = cfg.find("Shader");
    if (!fxNode) {
        return nullptr;
    }

    Ref<ShaderEffect> fx(new ShaderEffect());
    fx->setName(fxNode->name);
    auto it = fxNode->getIterator("Pass");

    for (auto i = it.first; i != it.second; ++i) {
        if (auto s = loadShader(i->second)) {
            fx->add(s);
        }
    }

    return fx;
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