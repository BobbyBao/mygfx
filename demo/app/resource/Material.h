#pragma once
#include "Fwd.h"
#include "MathTypes.h"


namespace mygfx {

	class Shader;
	class ShaderResourceInfo;
	class Texture;

	class Material : public utils::RefCounted {
	public:
		Material() = default;
		Material(Shader* shader, const String& materialUniformName);
		
		void setShader(Shader* shader, const String& materialUniformName);

		void setParameter(const String& name, const vec3& v);
		void setParameter(const String& name, const vec4& v);
		void setParameter(const String& name, Texture* tex);

	protected:
		Ref<Shader> mShader;
		String mMaterialUniformName;
		Ref<ShaderResourceInfo> mShaderResourceInfo;
		ByteArray mMaterialData;
		mutable uint32_t mMaterialUniforms = 0;
	};

}