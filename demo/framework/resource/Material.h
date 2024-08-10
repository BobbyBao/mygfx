#pragma once
#include "core/Fwd.h"
#include "core/Maths.h"
#include "PipelineState.h"
#include "core/Resource.h"

namespace mygfx {

	class Shader;
	class ShaderResourceInfo;
	class Texture;

	class Material : public Resource {
	public:
		Material();
		Material(Shader* shader, const String& materialUniformName);
		~Material();
		
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
		mutable uint32_t mMaterialUniforms = 0xffffffff;
	};

}