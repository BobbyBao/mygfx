#include "Skybox.h"
#include "resource/Texture.h"
#include "resource/Shader.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "Scene.h"

namespace mygfx {
	
	Skybox::Skybox() {
	}

	Object* Skybox::createObject() {
		return new Skybox();
	}

	void Skybox::cloneProcess(Object* destNode) {
		Skybox* skybox = (Skybox*)destNode;
		skybox->mCubeMap = mCubeMap;
		skybox->mIrrMap = mIrrMap;
		skybox->mGGXLUT = mGGXLUT;
	}

	void Skybox::setCubeMap(Texture* tex) {
		mCubeMap = tex;
	}
	
	void Skybox::setIrrMap(Texture* tex) {
		mIrrMap = tex;
	}

	void Skybox::onAddToScene(Scene* scene) {

		Renderable::onAddToScene(scene);

		if (mMesh == nullptr) {
			auto mesh = Mesh::createFullScreen();
			DefineList macros;
			macros.add("LINEAR_OUTPUT");
			Ref<Shader> shader = Shader::fromFile("shaders/skybox.vert", "shaders/skybox.frag", &macros);
			shader->setVertexInput({});
			shader->setCullMode(CullMode::NONE);
			Ref<Material> material = makeRef<Material>(shader.get(), "MaterialUniforms");
			material->setShaderParameter("u_MipLevel", 0);
			material->setShaderParameter("u_EnvBlurNormalized", 1.0f);
			mesh->setMaterial(material);
			setMesh(mesh);
		}

		if (mGGXLUT == nullptr) {
			mGGXLUT = Texture::createFromFile("textures/lut_ggx.png", SamplerInfo::create(Filter::LINEAR, SamplerAddressMode::CLAMP_TO_EDGE));
		}

		scene->skybox = this;
	}

	void Skybox::onRemoveFromScene(Scene* scene) {

		Renderable::onRemoveFromScene(scene);

		if (scene->skybox == this) {
			scene->skybox = nullptr;
		}
	}
}