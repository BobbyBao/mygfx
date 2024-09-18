#include "Shader.h"
#include "GraphicsApi.h"
#include "Texture.h"
#include "utils/FileUtils.h"
#include "ShaderCompiler.h"

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

Shader::Shader(const String& csCode)
{
    addShader(ShaderStage::COMPUTE, "", csCode, ShaderSourceType::GLSL, "", "", nullptr);
    init();
}

void Shader::loadShader(const String& vs, const String& fs, const DefineList* macros)
{
    Path vsPath(vs);
    auto vsSource = FileUtils::readAllText(vsPath);
    if (vsSource.empty()) {
        return;
    }

    FileUtils::pushPath(vsPath.parent_path());
    addShader(ShaderStage::VERTEX, vsPath.filename().string(), vsSource, ShaderSourceType::GLSL, "", "", macros);
    FileUtils::popPath();

    Path fsPath(fs);
    auto psSource = FileUtils::readAllText(fsPath);
    if (psSource.empty()) {
        return;
    }

    FileUtils::pushPath(fsPath.parent_path());
    addShader(ShaderStage::FRAGMENT, fsPath.filename().string(), psSource, ShaderSourceType::GLSL, "", "", macros);
    FileUtils::popPath();

    init();
}

void Shader::loadShader(const String& cs)
{
    Path csPath(cs);
    auto csSource = FileUtils::readAllText(csPath);
    if (csSource.empty()) {
        return;
    }

    FileUtils::pushPath(csPath.parent_path());
    addShader(ShaderStage::COMPUTE, csPath.filename().string(), csSource, ShaderSourceType::GLSL, "", "", nullptr);
    FileUtils::popPath();

    init();
}

void Shader::setVertexInput(const FormatList& fmts, const FormatList& fmts1)
{
    mVertexInput = gfxApi().createVertexInput(fmts, fmts1);

    if (mProgram) {
        mProgram->vertexInput = mVertexInput;
    }
}

void Shader::setVertexSemantic(VertexAttribute vertexSemantic)
{
    pipelineState.vertexSemantic = vertexSemantic;
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
    auto ds = mProgram->getDescriptorSet(set);
    if (ds != nullptr) {
        gfxApi().updateDescriptorSet1(ds, binding, tex->getSRV());
    }
}

void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, HwTextureView* texView)
{
    auto ds = mProgram->getDescriptorSet(set);
    if (ds != nullptr) {
        gfxApi().updateDescriptorSet1(ds, binding, texView);
    }
}

void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, HwBuffer* buffer)
{
    auto ds = mProgram->getDescriptorSet(set);
    if (ds != nullptr) {
        gfxApi().updateDescriptorSet2(ds, binding, buffer);
    }
}

void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, const BufferInfo& bufferInfo)
{
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