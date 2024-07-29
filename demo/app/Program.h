#pragma once
#include "GraphicsHandles.h"
#include "PipelineState.h"
#include "ShaderResourceInfo.h"

namespace mygfx {

	class Program : public RefCounted {
	public:
		Program();
		~Program();

		Program(const String& vsCode, const String& fsCode, const DefineList* marcos = nullptr);
		Program(const String& csCode);

		void loadShader(const String& vs, const String& fs, const DefineList* marcos = nullptr);
		void loadShader(const String& cs);

		inline const std::vector<Ref<HwShaderModule>>& shaderModules() const { return mShaderModules; }
		inline VertexAttribute getVertexSemantic() const { return pipelineState.vertexSemantic; }
		
		void setVertexInput(const FormatList& fmts, const FormatList& fmts1 = {});
		void setVertexSemantic(VertexAttribute vertexSemantic);
		void setBlendMode(BlendMode blendMode);
		void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

		PipelineState pipelineState;
		
	protected:
		void init();
		bool addShader(ShaderStage shaderStage, const String& source, ShaderSourceType sourceType, const String& entry, const String& extraParams, const DefineList* macros = nullptr);

		std::vector<Ref<HwShaderModule>> mShaderModules;
		Ref<HwVertexInput> mVertexInput;
		Ref<HwProgram> mProgram;		
	};


}
