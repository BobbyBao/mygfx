#include "Skybox.h"
#include "resource/Shader.h"
#include "resource/Material.h"
#include "resource/Mesh.h"

namespace mygfx {
	
	Skybox::Skybox() {
	}

	void Skybox::addToScene() {
		Renderable::addToScene();

		if (mMesh == nullptr) {
			auto mesh = Mesh::createCube(80.0f, VertexAttribute::POSITION);
			Ref<Shader> shader = Shader::fromFile("shaders/cubemap.vert", "shaders/cubemap.frag");
			shader->setVertexInput({ Format::R32G32B32_SFLOAT });
			shader->setCullMode(CullMode::NONE);
			shader->setDepthTest(true, false);
			Ref<Material> material = makeRef<Material>(shader.get(), "");
			mesh->setMaterial(material);
			
			setMesh(mesh);

		}
	}

	void Skybox::removeFromScene() {
		Renderable::removeFromScene();

	}
}