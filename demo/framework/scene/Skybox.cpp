#include "Skybox.h"
#include "resource/Shader.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "Scene.h"

namespace mygfx {
	
	Skybox::Skybox() {
	}

	void Skybox::setCubeMap(Texture* tex) {
		mCubeMap = tex;
	}

	void Skybox::addToScene() {
		Renderable::addToScene();

		if (mMesh == nullptr) {
			auto mesh = Mesh::createFullScreen();
			DefineList macros;
			macros.add("LINEAR_OUTPUT");
			Ref<Shader> shader = Shader::fromFile("shaders/skybox.vert", "shaders/skybox.frag", &macros);
			shader->setVertexInput({});
			shader->setCullMode(CullMode::NONE);
			Ref<Material> material = makeRef<Material>(shader.get(), "MaterialUniforms");
			material->setShaderParameter("u_MipLevel", 0);
			mesh->setMaterial(material);
			setMesh(mesh);
		}

		mScene->skybox = this;
	}

	void Skybox::removeFromScene() {
		Renderable::removeFromScene();

		if (mScene->skybox == this) {
			mScene->skybox = nullptr;
		}
	}
}