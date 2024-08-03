#include "Renderable.h"
#include "resource/Mesh.h"
#include "GraphicsApi.h"
#include "Scene.h"

namespace mygfx {
	
	Renderable::Renderable() = default;

	void Renderable::setMesh(Mesh* m) {

		mMesh = m;
		primitives.clear();
		
		for (uint32_t i = 0; i < mMesh->getSubMeshCount(); i++) {
			auto& subMesh = mMesh->getSubMeshAt(i);
			auto& primitive = primitives.emplace_back();
			primitive.renderPrimitive = mMesh->renderPrimitives[i];
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