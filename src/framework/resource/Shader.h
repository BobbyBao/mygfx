#pragma once
#include "GraphicsHandles.h"
#include "PipelineState.h"
#include "ShaderResourceInfo.h"
#include "core/Resource.h"

namespace mygfx {

class Texture;
class GraphicsApi;

class Shader : public Resource {
public:
    Shader();
    ~Shader();

    Shader(const String& vsCode, const String& fsCode, const DefineList* marcos = nullptr);
    Shader(const String& csCode);

    void loadShader(const String& vs, const String& fs, const DefineList* marcos = nullptr);
    void loadShader(const String& cs);

    const std::vector<Ref<HwShaderModule>>& getShaderModules() const { return mShaderModules; }
    VertexAttribute getVertexSemantic() const { return pipelineState.vertexSemantic; }
    HwProgram* getProgram() { return mProgram; }

    void setVertexInput(const FormatList& fmts, const FormatList& fmts1 = {});
    void setVertexSemantic(VertexAttribute vertexSemantic);
    void setBlendMode(BlendMode blendMode);
    void setPrimitiveTopology(PrimitiveTopology primitiveTopology);
    void setCullMode(CullMode cullMode);
    void setDepthTest(bool test, bool write);

    void updateDescriptorSet(uint32_t set, uint32_t binding, Texture* tex);
    void updateDescriptorSet(uint32_t set, uint32_t binding, HwTextureView* texView);
    void updateDescriptorSet(uint32_t set, uint32_t binding, HwBuffer* buffer);
    void updateDescriptorSet(uint32_t set, uint32_t binding, const BufferInfo& bufferInfo);

    PipelineState pipelineState;

    static Ref<Shader> fromFile(const String& vs, const String& fs, const DefineList* marcos = nullptr);

protected:
    void init();
    bool addShader(ShaderStage shaderStage, const String& shaderName, const String& source, ShaderSourceType sourceType, const String& entry, const String& extraParams, const DefineList* macros = nullptr);

    std::vector<Ref<HwShaderModule>> mShaderModules;
    Ref<HwVertexInput> mVertexInput;
    Ref<HwProgram> mProgram;
};

}
