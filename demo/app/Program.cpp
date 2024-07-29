#include "Program.h"
#include "GraphicsDevice.h"
#include "utils/FileUtils.h"

namespace mygfx
{

	Program::Program()
	{
	}

	Program::~Program()
	{
	}

	void Program::init() {

		mProgram = device().createProgram(mShaderModules.data(), (uint32_t)mShaderModules.size());
		mProgram->vertexInput = mVertexInput;
		pipelineState.program = mProgram;
		
	}

	bool Program::addShader(ShaderStage shaderStage, const String& source, ShaderSourceType sourceType, const String& entry, const String& extraParams, const DefineList* macros)
	{
		auto sm = device().compileShaderModule(sourceType, shaderStage, source, entry.c_str(), extraParams.c_str(), macros);
		if (sm == nullptr) {
			assert(false);
			return false;
		}

		mShaderModules.push_back(sm);
		return true;
	}

	void Program::create(const String& vsCode, const String& psCode, const DefineList* macros) {	
		addShader(ShaderStage::Vertex, vsCode, ShaderSourceType::GLSL, "", "", macros);
		addShader(ShaderStage::Fragment, psCode, ShaderSourceType::GLSL, "", "", macros);
		init();
	}

	void Program::create(const String& csCode) {
		addShader(ShaderStage::Compute, csCode, ShaderSourceType::GLSL, "", "", nullptr);
		init();
	}

	void Program::loadShader(const String& vs, const String& ps, const DefineList* macros) {
		auto vsSource = FileUtils::readAllText(vs);

		addShader(ShaderStage::Vertex, vsSource, ShaderSourceType::GLSL, "", "", macros);

		auto psSource = FileUtils::readAllText(ps);
		addShader(ShaderStage::Fragment, psSource, ShaderSourceType::GLSL, "", "", macros);

		init();
	}

	void Program::loadShader(const String& cs) {
		auto csSource = FileUtils::readAllText(cs);
		addShader(ShaderStage::Compute, csSource, ShaderSourceType::GLSL, "", "", nullptr);

		init();
	}
		
	void Program::setVertexInput(const FormatList& fmts, const FormatList& fmts1) {
		mVertexInput = device().createVertexInput(fmts, fmts1);

		if (mProgram) {
			mProgram->vertexInput = mVertexInput;
		}
	}

	void Program::setVertexSemantic(VertexAttribute vertexSemantic) {
		pipelineState.vertexSemantic = vertexSemantic;
	}
	
	void Program::setBlendMode(BlendMode blendMode) {
		pipelineState.colorBlendState = ColorBlendState::get(blendMode);
	}
	
	void Program::setPrimitiveTopology(PrimitiveTopology primitiveTopology) {
		pipelineState.primitiveState.primitiveTopology = primitiveTopology;
	}
}