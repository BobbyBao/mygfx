#include "Skybox.h"
#include "resource/Texture.h"
#include "resource/Shader.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "Scene.h"

namespace mygfx {
	
	Skybox::Skybox() {
	}

	Node* Skybox::createNode() {
		return new Skybox();
	}

	void Skybox::cloneProcess(Node* destNode) {
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
			material->setShaderParameter("u_EnvBlurNormalized", 1.0f);
			mesh->setMaterial(material);
			setMesh(mesh);
		}

		if (mGGXLUT == nullptr) {
			mGGXLUT = Texture::createFromFile("textures/brdfLut.dds", SamplerInfo{ .srgb = true });
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