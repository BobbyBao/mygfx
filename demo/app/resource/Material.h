#pragma once
#include "Fwd.h"
#include "MathTypes.h"
#include "PipelineState.h"

namespace mygfx {

	class Shader;
	class ShaderResourceInfo;
	class Texture;

	class Material : public utils::RefCounted {
	public:
		Material() = default;
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
	protected:
		Ref<Shader> mShader;
		PipelineState mPipelineState;
		String mMaterialUniformName;
		Ref<ShaderResourceInfo> mShaderResourceInfo;
		ByteArray mMaterialData;
		mutable uint32_t mMaterialUniforms = 0;
	};

}