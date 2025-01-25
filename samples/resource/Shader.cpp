#include "Shader.h"
#include "GraphicsApi.h"
#include "ShaderCompiler.h"
#include "Texture.h"
#include "FileSystem.h"

namespace mygfx {

Shader::Shader()
{
}

Shader::~Shader()
{
}

void Shader::init()
{
    mProgram = gfxApi().createProgram(mShaderModules.data(), (uint32_t)mShaderModules.size());
    mProgram->vertexInput = mVertexInput;
    pipelineState.program = mProgram;
}

bool Shader::addShader(ShaderStage shaderStage, const String& shaderName, const String& source, ShaderSourceType sourceType, const String& entry, const String& extraParams, const DefineList* macros)
{
    auto sm = ShaderCompiler::compileFromString(sourceType, shaderStage, shaderName, source, entry.c_str(), extraParams.c_str(), macros);
    if (sm == nullptr) {
        assert(false);
        return false;
    }

    mShaderModules.push_back(sm);
    return true;
}

Shader::Shader(const String& vsCode, const String& psCode, const DefineList* macros)
{
    addShader(ShaderStage::VERTEX, "", vsCode, ShaderSourceType::GLSL, "", "", macros);
    addShader(ShaderStage::FRAGMENT, "", psCode, ShaderSourceType::GLSL, "", "", macros);
    init();
}

Shader::Shader(const String& csCode, const DefineList* macros)
{
    addShader(ShaderStage::COMPUTE, "", csCode, ShaderSourceType::GLSL, "", "", macros);
    init();
}

void Shader::setName(const std::string_view& name)
{
    passName = name;
}

void Shader::loadShader(const String& vs, const String& fs, const DefineList* macros)
{
    auto vsSource = FileSystem::readAllText(vs);
    if (vsSource.empty()) {
        return;
    }

    addShader(ShaderStage::VERTEX, vs, vsSource, ShaderSourceType::GLSL, "", "", macros);

    auto psSource = FileSystem::readAllText(fs);
    if (psSource.empty()) {
        return;
    }

    addShader(ShaderStage::FRAGMENT, fs, psSource, ShaderSourceType::GLSL, "", "", macros);

    init();
}

void Shader::loadShader(const String& cs, const DefineList* macros)
{
    Path csPath(cs);
    auto csSource = FileSystem::readAllText(csPath);
    if (csSource.empty()) {
        return;
    }

    addShader(ShaderStage::COMPUTE, cs, csSource, ShaderSourceType::GLSL, "", "", macros);

    init();
}

ShaderResourceInfo* Shader::getShaderResource(const String& name)
{
    return mProgram->getShaderResource(name);
}

void Shader::setVertexInput(const FormatList& fmts, const FormatList& fmts1)
{
    mVertexInput = gfxApi().createVertexInput(fmts, fmts1);

    if (mProgram) {
        mProgram->vertexInput = mVertexInput;
    }
}

void Shader::setBlendMode(BlendMode blendMode)
{
    pipelineState.colorBlendState = ColorBlendState::get(blendMode);
}

void Shader::setPrimitiveTopology(PrimitiveTopology primitiveTopology)
{
    pipelineState.primitiveState.primitiveTopology = primitiveTopology;
}

void Shader::setCullMode(CullMode cullMode)
{
    pipelineState.rasterState.cullMode = cullMode;
}

void Shader::setDepthTest(bool test, bool write)
{
    pipelineState.depthState.depthTestEnable = test;
    pipelineState.depthState.depthWriteEnable = write;
}

void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, Texture* tex)
{
    CHECK_MAIN_THREAD();
    auto ds = mProgram->getDescriptorSet(set);
    if (ds != nullptr) {
        gfxApi().updateDescriptorSet1(ds, binding, tex->getSRV());
    }
}

void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, HwTextureView* texView)
{
    CHECK_MAIN_THREAD();
    auto ds = mProgram->getDescriptorSet(set);
    if (ds != nullptr) {
        gfxApi().updateDescriptorSet1(ds, binding, texView);
    }
}

void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, HwBuffer* buffer)
{
    CHECK_MAIN_THREAD();
    auto ds = mProgram->getDescriptorSet(set);
    if (ds != nullptr) {
        gfxApi().updateDescriptorSet2(ds, binding, buffer);
    }
}

void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, const BufferInfo& bufferInfo)
{
    CHECK_MAIN_THREAD();
    auto ds = mProgram->getDescriptorSet(set);
    if (ds != nullptr) {
        gfxApi().updateDescriptorSet3(ds, binding, bufferInfo);
    }
}

Ref<Shader> Shader::fromFile(const String& vs, const String& fs, const DefineList* marcos)
{
    Ref<Shader> shader(new Shader());
    shader->loadShader(vs, fs, marcos);
    return shader;
}
}