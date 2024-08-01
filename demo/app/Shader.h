#pragma once
#include "GraphicsHandles.h"
#include "PipelineState.h"
#include "ShaderResourceInfo.h"

namespace mygfx {
	
	class Texture;
	class GraphicsApi;

	class Shader : public RefCounted {
	public:
		Shader();
		~Shader();

		Shader(const String& vsCode, const String& fsCode, const DefineList* marcos = nullptr);
		Shader(const String& csCode);

		void loadShader(const String& vs, const String& fs, const DefineList* marcos = nullptr);
		void loadShader(const String& cs);

		const std::vector<Ref<HwShaderModule>>& shaderModules() const { return mShaderModules; }
		VertexAttribute getVertexSemantic() const { return pipelineState.vertexSemantic; }
		HwProgram* getProgram() { return mProgram; }

		void setVertexInput(const FormatList& fmts, const FormatList& fmts1 = {});
		void setVertexSemantic(VertexAttribute vertexSemantic);
		void setBlendMode(BlendMode blendMode);
		void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

		void updateDescriptorSet(uint32_t set, uint32_t binding, Texture* tex);
		void updateDescriptorSet(uint32_t set, uint32_t binding, HwTextureView* texView);
		void updateDescriptorSet(uint32_t set, uint32_t binding, HwBuffer* buffer);
		void updateDescriptorSet(uint32_t set, uint32_t binding, const BufferInfo& bufferInfo);

		PipelineState pipelineState;
		
	protected:
		void init();
		bool addShader(ShaderStage shaderStage, const String& source, ShaderSourceType sourceType, const String& entry, const String& extraParams, const DefineList* macros = nullptr);

		std::vector<Ref<HwShaderModule>> mShaderModules;
		Ref<HwVertexInput> mVertexInput;
		Ref<HwProgram> mProgram;		
	};


}
