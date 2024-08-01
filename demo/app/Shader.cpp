#include "Shader.h"
#include "GraphicsDevice.h"
#include "utils/FileUtils.h"
#include "Texture.h"
#include "GraphicsApi.h"

namespace mygfx
{

	Shader::Shader()
	{
	}

	Shader::~Shader()
	{
	}

	void Shader::init() {

		mProgram = device().createProgram(mShaderModules.data(), (uint32_t)mShaderModules.size());
		mProgram->vertexInput = mVertexInput;
		pipelineState.program = mProgram;
		
	}

	bool Shader::addShader(ShaderStage shaderStage, const String& source, ShaderSourceType sourceType, const String& entry, const String& extraParams, const DefineList* macros)
	{
		auto sm = device().compileShaderModule(sourceType, shaderStage, source, entry.c_str(), extraParams.c_str(), macros);
		if (sm == nullptr) {
			assert(false);
			return false;
		}

		mShaderModules.push_back(sm);
		return true;
	}

	Shader::Shader(const String& vsCode, const String& psCode, const DefineList* macros) {	
		addShader(ShaderStage::Vertex, vsCode, ShaderSourceType::GLSL, "", "", macros);
		addShader(ShaderStage::Fragment, psCode, ShaderSourceType::GLSL, "", "", macros);
		init();
	}

	Shader::Shader(const String& csCode) {
		addShader(ShaderStage::Compute, csCode, ShaderSourceType::GLSL, "", "", nullptr);
		init();
	}

	void Shader::loadShader(const String& vs, const String& ps, const DefineList* macros) {
		auto vsSource = FileUtils::readAllText(vs);

		addShader(ShaderStage::Vertex, vsSource, ShaderSourceType::GLSL, "", "", macros);

		auto psSource = FileUtils::readAllText(ps);
		addShader(ShaderStage::Fragment, psSource, ShaderSourceType::GLSL, "", "", macros);

		init();
	}

	void Shader::loadShader(const String& cs) {
		auto csSource = FileUtils::readAllText(cs);
		addShader(ShaderStage::Compute, csSource, ShaderSourceType::GLSL, "", "", nullptr);

		init();
	}
		
	void Shader::setVertexInput(const FormatList& fmts, const FormatList& fmts1) {
		mVertexInput = device().createVertexInput(fmts, fmts1);

		if (mProgram) {
			mProgram->vertexInput = mVertexInput;
		}
	}

	void Shader::setVertexSemantic(VertexAttribute vertexSemantic) {
		pipelineState.vertexSemantic = vertexSemantic;
	}
	
	void Shader::setBlendMode(BlendMode blendMode) {
		pipelineState.colorBlendState = ColorBlendState::get(blendMode);
	}
	
	void Shader::setPrimitiveTopology(PrimitiveTopology primitiveTopology) {
		pipelineState.primitiveState.primitiveTopology = primitiveTopology;
	}

	void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, Texture* tex) {
		auto ds = mProgram->getDescriptorSet(set);
		if (ds != nullptr) {
			gfxApi().updateDescriptorSet1(ds, binding, tex->getSRV());
		}
	}

	void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, HwTextureView* texView) {

		auto ds = mProgram->getDescriptorSet(set);	
		if (ds != nullptr) {
			gfxApi().updateDescriptorSet1(ds, binding, texView);
		}
	}

	void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, HwBuffer* buffer) {

		auto ds = mProgram->getDescriptorSet(set);	
		if (ds != nullptr) {
			gfxApi().updateDescriptorSet2(ds, binding, buffer);
		}
	}

	void Shader::updateDescriptorSet(uint32_t set, uint32_t binding, const BufferInfo& bufferInfo) {

		auto ds = mProgram->getDescriptorSet(set);	
		if (ds != nullptr) {
			gfxApi().updateDescriptorSet3(ds, binding, bufferInfo);
		}
	}
}