#pragma once
#include "Fwd.h"
#include "core/MathTypes.h"
#include "PipelineState.h"

namespace mygfx {

	class Shader;
	class ShaderResourceInfo;
	class Texture;

	class Material : public utils::RefCounted {
	public:
		Material();
		Material(Shader* shader, const String& materialUniformName);
		
		void setShader(Shader* shader, const String& materialUniformName);
		
		void setShaderParameter(const String& name, int v);
		void setShaderParameter(const String& name, float v);
		void setShaderParameter(const String& name, const vec3& v);
		void setShaderParameter(const String& name, const vec4& v);
		void setShaderParameter(const String& name, Texture* tex);
		
		void setDoubleSide(bool v);
		void setWireframe(bool v);
		void setBlendMode(BlendMode blendMode);

		inline Shader* shader() { return mShader; }
		const PipelineState& getPipelineState() const { return mPipelineState; }
		inline uint32_t getMaterialUniforms() const {	
			return mMaterialUniforms;		
		}

		static void updateAll();
	protected:
		void update();
		Ref<Shader> mShader;
		PipelineState mPipelineState;
		String mMaterialUniformName;
		Ref<ShaderResourceInfo> mShaderResourceInfo;
		ByteArray mMaterialData;
		mutable uint32_t mMaterialUniforms = 0;
	};

}