#include "Renderable.h"
#include "resource/Mesh.h"
#include "GraphicsApi.h"
#include "Scene.h"

namespace mygfx {
	
	Renderable::Renderable() = default;

	void Renderable::setMesh(Mesh* m) {

		mesh = m;
		primitives.clear();
		
		for (uint32_t i = 0; i < mesh->getSubMeshCount(); i++) {
			auto& subMesh = mesh->getSubMeshAt(i);
			auto& primitive = primitives.emplace_back();
			primitive.renderPrimitive = mesh->renderPrimitives[i];
			primitive.material = subMesh.material;
		}
	}
		
	void Renderable::addToScene() {
		mScene->renderables.insert(this);
	}

	void Renderable::removeFromScene() {
		mScene->renderables.erase(this);
	}
}