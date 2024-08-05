#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "GraphicsApi.h"

namespace mygfx {

	static HashSet<Material*> sMaterials;
	static std::recursive_mutex sLock;

	Material::Material() {
		sMaterials.insert(this);
	}

	Material::Material(Shader* shader, const String& materialUniformName) {
		sMaterials.insert(this);

		setShader(shader, materialUniformName);
	}

	Material::~Material() {
		sMaterials.erase(this);
	}

	void Material::setShader(Shader* shader, const String& materialUniformName) {
		if (mShader == shader || shader == nullptr) {
			return;
		}

		mShader = shader;
		mMaterialUniformName = materialUniformName;	
		mPipelineState = mShader->pipelineState;
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

	void Material::update() {
		if (mMaterialData.empty()) {
			return;
		}
		
		void* pData;
		BufferInfo bufferInfo;
		if (!device().allocConstantBuffer((uint32_t)mMaterialData.size(), &pData, &bufferInfo)) {
			return;
		}

		std::memcpy(pData, mMaterialData.data(), mMaterialData.size());
		mMaterialUniforms = bufferInfo.offset;
	}
	
	void Material::updateAll() {
		for (auto material : sMaterials) {
			material->update();
		}
	}
}