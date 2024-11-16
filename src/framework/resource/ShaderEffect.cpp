#include "ShaderEffect.h"
#include "Shader.h"
#include "core/Config.h"
#include "core/FileSystem.h"
#include "magic_enum/magic_enum.hpp"
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
    for (auto& shader : mShaderPasses) {
        if (shader.second->getName() == name) {
            return shader.second;
        }
    }
    return nullptr;
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
    mShaderPasses.emplace_back(shader->getName(), shader);
}

static bool loadShader(Shader* shader, const ConfigValue& shaderNode)
{
    DefineList defines;
    if (auto macros = shaderNode.find("Macros")) {
        auto it = macros->getIterator();
        for (auto i = it.first; i != it.second; ++i) {
            defines.add(String(i->first), i->second.getString());
        }
    }

    if (auto cs = shaderNode.find("ComputeShader")) {
        shader->loadShader(cs->getString(), &defines);
    }

    if (auto vs = shaderNode.find("VertexShader")) {
        if (auto fs = shaderNode.find("PixelShader")) {
            shader->loadShader(vs->getString(), fs->getString(), &defines);
        }
    }

    if (auto cs = shaderNode.find("@ComputeShader")) {
        ShaderSourceType shaderSourceType = cs->name == "HLSL" ? ShaderSourceType::HLSL : ShaderSourceType::GLSL;
        shader->addShader(ShaderStage::COMPUTE, "", cs->getString(), shaderSourceType, "", "", &defines);
    }

    if (auto vs = shaderNode.find("@VertexShader")) {
        ShaderSourceType shaderSourceTypeVS = vs->name == "HLSL" ? ShaderSourceType::HLSL : ShaderSourceType::GLSL;
        if (auto fs = shaderNode.find("@PixelShader")) {
            ShaderSourceType shaderSourceTypeFS = fs->name == "HLSL" ? ShaderSourceType::HLSL : ShaderSourceType::GLSL;
            shader->addShader(ShaderStage::VERTEX, "", vs->getString(), shaderSourceTypeVS, "", "", &defines);
            shader->addShader(ShaderStage::FRAGMENT, "", fs->getString(), shaderSourceTypeFS, "", "", &defines);
        }
    }

    if (auto vertexInput = shaderNode.find("VertexInput")) {

        FormatList fmts1, fmts2;
        if (auto perVertex = vertexInput->find("PerVertex")) {
            for (auto i = 0; i < perVertex->getArrayCount(); i++) {
                auto& e = perVertex->getAt(i);
                auto& val = e.get<std::string_view>();
                auto fmt = magic_enum::enum_cast<Format>(val);
                if (fmt.has_value())
                    fmts1.push_back(fmt.value());
                else
                    LOG_ERROR("Illegal format value :{}", val);
            }
        }

        if (auto perInstance = vertexInput->find("PerInstance")) {
            for (auto i = 0; i < perInstance->getArrayCount(); i++) {
                auto& e = perInstance->getAt(i);
                auto& val = e.get<std::string_view>();
                auto fmt = magic_enum::enum_cast<Format>(val);
                if (fmt.has_value())
                    fmts2.push_back(fmt.value());
                else
                    LOG_ERROR("Unknown format value :{}", val);
            }
        }

        shader->setVertexInput(fmts1, fmts2);
    }

    if (auto node = shaderNode.find("PrimitiveTopology")) {
        auto& str = node->get<std::string_view>();
        auto e = magic_enum::enum_cast<PrimitiveTopology>(str);
        if (e.has_value())
            shader->setPrimitiveTopology(e.value());
        else
            LOG_ERROR("Illegal PrimitiveTopology :{}", str);
    }

    if (auto node = shaderNode.find("CullMode")) {
        auto& str = node->get<std::string_view>();
        auto e = magic_enum::enum_cast<CullMode>(str);
        if (e.has_value())
            shader->setCullMode(e.value());
        else
            LOG_ERROR("Illegal CullMode :{}", str);
    }

    if (auto node = shaderNode.find("BlendMode")) {
        auto& str = node->get<std::string_view>();
        auto e = magic_enum::enum_cast<BlendMode>(str);
        if (e.has_value())
            shader->setBlendMode(e.value());
        else
            LOG_ERROR("Illegal BlendMode :{}", str);
    }

    bool depthTest = true;
    bool depthWrite = true;
    if (auto node = shaderNode.find("DepthTest")) {
        depthTest = node->get<bool>(depthTest);
    }

    if (auto node = shaderNode.find("DepthWrite")) {
        depthWrite = node->get<bool>(depthWrite);
    }

    shader->setDepthTest(depthTest, depthWrite);

    if (auto blendState = shaderNode.find("BlendState")) {
    }

    if (auto depthState = shaderNode.find("DepthState")) {
    }

    if (auto stencilState = shaderNode.find("StencilState")) {
    }

    shader->init();
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