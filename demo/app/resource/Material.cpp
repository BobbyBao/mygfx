#include "Material.h"
#include "Shader.h"
#include "Texture.h"

namespace mygfx {
	
	Material::Material(Shader* shader, const String& materialUniformName) {
		setShader(shader, materialUniformName);
	}
	
	void Material::setShader(Shader* shader, const String& materialUniformName) {
		if (mShader == shader || shader == nullptr) {
			return;
		}

		mShader = shader;
		mMaterialUniformName = materialUniformName;
		mShaderResourceInfo = shader->getProgram()->getShaderResource(materialUniformName);

		if (mShaderResourceInfo) {
			mMaterialData.resize(mShaderResourceInfo->getMemberSize());
		}
	}
	
	void Material::setShaderParameter(const String& name, int v) {
		
		if (mShaderResourceInfo) {
			auto member = mShaderResourceInfo->getMember(name);
			if (member) {
				assert(member->size == sizeof(int));
				std::memcpy(&mMaterialData[member->offset], &v, sizeof(int));
			}
		}
	}

	void Material::setShaderParameter(const String& name, float v) {
		
		if (mShaderResourceInfo) {
			auto member = mShaderResourceInfo->getMember(name);
			if (member) {
				assert(member->size == sizeof(float));
				std::memcpy(&mMaterialData[member->offset], &v, sizeof(float));
			}
		}
	}
	
	void Material::setShaderParameter(const String& name, const vec3& v) {
		
		if (mShaderResourceInfo) {
			auto member = mShaderResourceInfo->getMember(name);
			if (member) {
				assert(member->size == sizeof(vec3));
				std::memcpy(&mMaterialData[member->offset], &v, sizeof(vec3));
			}
		}
	}

	void Material::setShaderParameter(const String& name, const vec4& v) {
		
		if (mShaderResourceInfo) {
			auto member = mShaderResourceInfo->getMember(name);
			if (member) {
				assert(member->size == sizeof(vec4));
				std::memcpy(&mMaterialData[member->offset], &v, sizeof(vec4));
			}
		}
	}

	void Material::setShaderParameter(const String& name, Texture* tex)
	{
		if (mShaderResourceInfo) {
			auto member = mShaderResourceInfo->getMember(name);
			if (member) {
				int32_t texIndex = tex->getSRV()->index();
				assert(member->size == sizeof(int32_t));
				std::memcpy(&mMaterialData[member->offset], &texIndex, sizeof(int32_t));
			}
		}

	}

	void Material::setDoubleSide(bool v) {
		mPipelineState.rasterState.cullMode = v ? CullMode::NONE : CullMode::BACK;
	}
	
	void Material::setWireframe(bool v) {
		mPipelineState.rasterState.polygonMode = v ? PolygonMode::LINE : PolygonMode::FILL;
	}

	void Material::setBlendMode(BlendMode blendMode) {
		mPipelineState.colorBlendState = ColorBlendState::get(blendMode);
	}
	
}