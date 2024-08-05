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
			auto mesh = Mesh::createCube(80.0f, VertexAttribute::POSITION);
			Ref<Shader> shader = Shader::fromFile("shaders/cubemap.vert", "shaders/cubemap.frag");
			shader->setVertexInput({ Format::R32G32B32_SFLOAT });
			shader->setCullMode(CullMode::BACK);
			shader->setDepthTest(true, false);
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